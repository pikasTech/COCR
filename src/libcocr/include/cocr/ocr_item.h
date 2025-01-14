#pragma once

#include "els_cocr_export.h"
#include "base/cocr_types.h"
#include "ckit/atom.h"
#include "ckit/bond.h"
#include "ocv/mat.h"
#include <string>

struct OCRDataItem;

class ELS_COCR_EXPORT OCRItem {
    std::shared_ptr<OCRDataItem> data;
    size_t uid;
public:
    OCRItemType type;

    OCRItem(const size_t &_uid = 0);

    void setUId(const size_t &_uid);

    size_t getUId() const;

    void setAsLineBond(const BondType &_bt, const rectf &_rect, const Mat &_input);

    void setAsCircleBond(const rectf &_rect);

    void setAsText(std::string &_text, const rectf &_rect);

    ElementType getElement() const;

    const point2f &getFrom() const;

    const point2f &getTo() const;

    float getRadius() const;

    std::string getText() const;

    const rectf &getRect() const;

    point2f getCenter() const;

    BondType getBondType() const;
};