#include "core/tk_menu_stack.h"
#include <QMenu>

MenuStack::MenuStack(const QString name) { _stack.push_back(new QMenu(name)); }

MenuStack::~MenuStack(void) {
    if(_ownsStack) {
        delete first();
    }
}

QMenu* MenuStack::push(const QString name) {
    auto* menu = _stack.size()? current()->addMenu(name) : new QMenu(name);
    _stack.push_back(menu);
    return menu;
}

QAction* MenuStack::insertAction(const QString text, const std::function<void(bool)>& callback, const bool enabled) {
    auto* action = new QAction(text, nullptr);
    //motherfucking C++11 lambda function Qt SLAWTS!!!
    QObject::connect(action, &QAction::triggered, callback);
    action->setEnabled(enabled);
    _stack.top()->addAction(action);
    return action;
}

QAction* MenuStack::insertCheckableAction(const QString text, const std::function<void(bool)>& callback, const bool checked, const bool enabled) {
    QAction* action = insertAction(text, callback, enabled);
    action->setCheckable(true);
    action->blockSignals(true);
    action->setChecked(checked);
    action->blockSignals(false);
    return action;
}
