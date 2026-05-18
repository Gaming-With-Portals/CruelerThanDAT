#pragma once
#include "BinaryHandler.h"
#include "WMBModel.h"
#include "WMB/Formats/WMB4.h"

class IWMBParser {
public:
	virtual ParseResult<WmbModel> Parse(BinaryReader& reader) = 0;
};


class WMB4Parser : public IWMBParser {
	ParseResult<WmbModel> Parse(BinaryReader& reader) override;
};