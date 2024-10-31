#ifndef PROGRESS
#define PROGRESS

namespace progress {
	void bar(const double& full_size, const double& occupied);

	void prepare(void);

	void ingoing(const double& full_size, const double& occupied);

	void reset(void);

	struct obj {
			obj() {
				progress::prepare();
			}

			~obj() {
				progress::reset();
			}
	};
}

#endif // PROGRESS
