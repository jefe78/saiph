#ifndef ACTION_LIST_INVENTORY_H
#define ACTION_LIST_INVENTORY_H

#include "World.h"
#include "Actions/Action.h"

namespace action {

	class ListInventory : public Action {
	public:
		static const int ID;

		ListInventory(analyzer::Analyzer* analyzer) : Action(analyzer, false), _list_inventory("i", PRIORITY_LIST_INVENTORY), _close_page(CLOSE_PAGE, PRIORITY_CONTINUE_ACTION) {
		}

		virtual ~ListInventory() {
		}

		virtual int id() {
			return ID;
		}

		virtual const Command& command() {
			switch (_sequence) {
			case 0:
				return _list_inventory;

			case 1:
				return _close_page;

			default:
				return Action::NOOP;
			}
		}

		virtual void update(const std::string&) {
			if (_sequence == 0)
				_sequence = 1;
			else if (_sequence == 1 && !World::menu())
				_sequence = 2;
		}

	private:
		const Command _list_inventory;
		const Command _close_page;
	};
}
#endif
