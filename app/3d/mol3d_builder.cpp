#include "mol3d_builder.hpp"

#include "jmol.hpp"
#include "math_util.hpp"

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>

#include <QDebug>

using xgd::MathUtil;

inline QVector3D getQVector3D(const xgd::JAtom &_atom) {
    return {_atom.xx, _atom.yy, _atom.zz};
}

inline QVector3D getQVector3D(const std::shared_ptr<xgd::JAtom> &_atom) {
    return getQVector3D(*_atom);
}


void Mol3DBuilder::prepare(std::shared_ptr<xgd::JMol> _mol, const QVector3D &_viewSize,
                           const QVector3D &_blankArea) {
    qDebug() << __FUNCTION__;
    mol = std::move(_mol);
    mol->norm3D(_viewSize.x(), _viewSize.y(), _viewSize.z(),
                _blankArea.x() / 2, _blankArea.y() / 2, _blankArea.z() / 2, true);
    emit sig_mol_prepare_done();
}

void Mol3DBuilder::build() {
    qDebug() << __FUNCTION__;
    using namespace xgd;
    // 坐标系
//    QVector3D offset(-baseSize / 3, baseSize / 3, 0);
//    getSingleCylinderEntity(offset + zeroP, offset + axisX * 10, 0.2, Qt::blue);
//    getSingleCylinderEntity(offset + zeroP, offset + axisY * 10, 0.2, Qt::red);
//    getSingleCylinderEntity(offset + zeroP, offset + axisZ * 10, 0.2, Qt::green);
    float avgBondLength = mol->getAvgBondLength();
    if (avgBondLength < 1)avgBondLength = 20;
    // 添加3D原子球
    mol->loopAtomVec([&](xgd::JAtom &atom) {
        auto wrapper = std::make_shared<SphereWrapper>(root);
        atoms[atom.getId()] = wrapper;
//        qDebug() << getQVector3D(atom);
        wrapper->setId(atom.getId());
        wrapper->setTranslation(getQVector3D(atom));
        wrapper->setColor(xgd::getColor(atom.getType()));
        wrapper->setRadius(atom.getRadius() / atom.getDefaultDadius() * avgBondLength / 3);
        wrapper->setScale(1);
        wrapper->setRindsAndSlices(100, 100);
        wrapper->setId(atom.getId());
    });
    // 统计共轭键的共面原子，计算用于修正双键的中轴旋转角度
    std::unordered_map<size_t, QVector3D> normVecMap;
    std::unordered_map<size_t, std::vector<size_t>> neighborMap;
    // 初始化原子的邻接原子
    mol->loopBondVec([&](JBond &bond) {
        auto from = bond.getFrom()->getId(), to = bond.getTo()->getId();
        neighborMap[from].push_back(to);
        neighborMap[to].push_back(from);
    });
    // 寻找需要共面的共轭键
    mol->loopBondVec([&](JBond &bond) {
        switch (bond.getType()) {
            case BondType::DoubleBond:
            case BondType::TripleBond:
                break;
            default:
                return;
        }
        size_t from = bond.getFrom()->getId(), to = bond.getTo()->getId();
        std::vector<QVector3D> poses;
        std::unordered_set<size_t> aids = {from, to};
        auto collect_neb_pos = [&](const size_t &aid) {
            auto it = neighborMap.find(aid);
            if (it == neighborMap.end()) { return; }
            for (auto &neb_id:it->second) {
                auto nebIt = aids.find(neb_id);
                if (aids.end() != nebIt) { continue; }
                aids.insert(neb_id);
                auto atom = mol->getAtom(neb_id);
                poses.push_back(getQVector3D(atom));
            }
        };
        collect_neb_pos(from);
        collect_neb_pos(to);
        // 对于三键，存在直连原子不足的问题，找邻居的邻居
        if (poses.size() < 3) {
            auto collect_neb_2_pos = [&](const size_t &aid) {
                auto it = neighborMap.find(aid);
                if (it == neighborMap.end()) { return; }
                for (auto &neb_id:it->second) {
                    auto it2 = neighborMap.find(neb_id);
                    if (it2 == neighborMap.end()) { continue; }
                    for (auto &neb_2_id:it2->second) {
                        auto nebIt = aids.find(neb_2_id);
                        if (aids.end() != nebIt) { continue; }
                        aids.insert(neb_2_id);
                        auto atom = mol->getAtom(neb_2_id);
                        poses.push_back(getQVector3D(atom));
                    }
                }
            };
            collect_neb_2_pos(from);
            collect_neb_2_pos(to);
        }
        if (poses.size() >= 3) {
            normVecMap[bond.getId()] = QVector3D::crossProduct(poses[0] - poses[1], poses[1] - poses[2]);
        }
    });
    float bondRadius = avgBondLength / 40;
    mol->loopBondVec([&](JBond &bond) {
        auto fromAtom = bond.getFrom(), toAtom = bond.getTo();
        QVector3D from = getQVector3D(fromAtom), to = getQVector3D(toAtom);
        std::shared_ptr<BaseWrapper> wrapper;
        switch (bond.getType()) {
            case BondType::SingleBond:
            case BondType::DelocalizedBond:
            case BondType::DownBond:
            case BondType::UpBond:
            case BondType::ImplicitBond: {
                auto cWrapper = std::make_shared<CylinderWrapper>(root);
                cWrapper->setDirection(from, to);
                cWrapper->setRadius(bondRadius);
                wrapper = cWrapper;
                break;
            }
            case BondType::DoubleBond: {
                auto mcWrapper = std::make_shared<MultiCylinderWrapper>(root, 2);
                std::optional<QVector3D> norm = std::nullopt;
                auto it = normVecMap.find(bond.getId());
                if (it != normVecMap.end()) {
                    norm = it->second;
                }
                mcWrapper->setDirection(from, to, norm);
                mcWrapper->setDistance(bondRadius * 2);
                mcWrapper->setRadius(bondRadius);
                wrapper = mcWrapper;
                break;
            }
            case BondType::TripleBond: {
                auto mcWrapper = std::make_shared<MultiCylinderWrapper>(root, 3);
                std::optional<QVector3D> norm = std::nullopt;
                auto it = normVecMap.find(bond.getId());
                if (it != normVecMap.end()) {
                    norm = it->second;
                }
                mcWrapper->setDirection(from, to, norm);
                mcWrapper->setDistance(bondRadius * 3);
                mcWrapper->setRadius(bondRadius);
                wrapper = mcWrapper;
                break;
            }
            default: {
                exit(-1);
            }
        }
        wrapper->setRindsAndSlices(100, 100);
        wrapper->setScale(1);
        wrapper->setColor(getColor(bond.getType()));
        wrapper->setId(bond.getId());
        bonds[bond.getId()] = wrapper;
    });
    emit sig_mol_build_done();
}