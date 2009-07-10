#ifndef ACTION_ANSWER_H
#define ACTION_ANSWER_H

#include "Action.h"

namespace action {
	class Answer : public Action {
	public:
		static int id;

		Answer(analyzer::Analyzer *analyzer, std::string answer) : Action(analyzer), answer(answer, PRIORITY_CONTINUE_ACTION) {}
		virtual ~Answer() {}

		virtual int getID() {return id;}
		virtual const Command &getCommand();
		virtual void updateAction(const std::string &messages);

	private:
		const Command answer;
	};

	inline const Command &Answer::getCommand() {
		switch (sequence) {
		case 0:
			return answer;

		default:
			return Action::noop;
		}
	}

	inline void Answer::updateAction(const std::string &) {
		if (sequence == 0)
			sequence = 1;
	}
}
#endif