template<O, E>
union ResultData {
	O ok;
	E er;
};

template<O, E>
struct Result {
	bool isOk;
	ResultData data;

	O UnwrapOr(const O default) {
		if (this.isOk) {
			return this.data.ok;
		}
		else {
			return default;
		}
	}
};
