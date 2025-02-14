﻿#ifndef VIEWMODELS_H
#define VIEWMODELS_H
#include "xml/ModuleConfiguration.h"


template <typename T>
using shared_ptr_list = std::vector<std::shared_ptr<T> >;

/*
--------------------------------------------------------------------------------
                                Plugins
--------------------------------------------------------------------------------
*/
class PluginViewModel {
public:
    PluginViewModel(const std::shared_ptr<Plugin>& plugin_, const bool selected, bool, const int index)
        : ownIndex(index), selected(selected), enabled(true), plugin(plugin_) {}

    void setSelected(const bool selected) { this->selected = selected; }
    void setEnabled(const bool enabled) { this->enabled = enabled; }
    [[nodiscard]] std::string getName() const { return plugin->name; }
    [[nodiscard]] std::string getDescription() const { return plugin->description; }
    [[nodiscard]] std::string getImagePath() const { return plugin->image.path; }
    [[nodiscard]] bool isSelected() const { return selected; }
    [[nodiscard]] bool isEnabled() const { return enabled; }
    [[nodiscard]] std::vector<ConditionFlag> getConditionFlags() const { return plugin->conditionFlags.flags; }
    int getOwnIndex() const { return ownIndex; }
    PluginTypeEnum getCurrentPluginType() const { return currentPluginType; }
    void setCurrentPluginType(const PluginTypeEnum type) { currentPluginType = type; }

    friend class FomodViewModel;
    friend class FileInstaller;
    friend class ConditionTester;

protected:
    [[nodiscard]] std::shared_ptr<Plugin> getPlugin() const { return plugin; }

private:
    int ownIndex;
    bool selected;
    bool enabled;
    PluginTypeEnum currentPluginType = PluginTypeEnum::UNKNOWN;
    std::shared_ptr<Plugin> plugin;
};

/*
--------------------------------------------------------------------------------
                                Groups
--------------------------------------------------------------------------------
*/
class GroupViewModel {
public:
    GroupViewModel(const std::shared_ptr<Group>& group_, const shared_ptr_list<PluginViewModel>& plugins,
        const int index, const int stepIndex)
        : plugins(plugins), group(group_), ownIndex(index), stepIndex(stepIndex) {}

    void addPlugin(const std::shared_ptr<PluginViewModel>& plugin) { plugins.emplace_back(plugin); }

    [[nodiscard]] std::string getName() const { return group->name; }
    [[nodiscard]] GroupTypeEnum getType() const { return group->type; }
    [[nodiscard]] shared_ptr_list<PluginViewModel> getPlugins() const { return plugins; }
    [[nodiscard]] int getOwnIndex() const { return ownIndex; }
    [[nodiscard]] int getStepIndex() const { return stepIndex; }

private:
    shared_ptr_list<PluginViewModel> plugins;
    std::shared_ptr<Group> group;
    int ownIndex;
    int stepIndex;
};

/*
--------------------------------------------------------------------------------
                                Steps
--------------------------------------------------------------------------------
*/
class StepViewModel {
public:
    StepViewModel(const std::shared_ptr<InstallStep>& installStep_, const shared_ptr_list<GroupViewModel>& groups,
        const int index)
        : installStep(installStep_), groups(groups), ownIndex(index) {}

    [[nodiscard]] CompositeDependency& getVisibilityConditions() const { return installStep->visible; }
    [[nodiscard]] std::string getName() const { return installStep->name; }
    [[nodiscard]] const shared_ptr_list<GroupViewModel>& getGroups() const { return groups; }
    [[nodiscard]] int getOwnIndex() const { return ownIndex; }
    [[nodiscard]] bool getHasVisited() const { return visited; }
    void setVisited(const bool visited) { this->visited = visited; }

private:
    bool visited{false};
    std::shared_ptr<InstallStep> installStep;
    shared_ptr_list<GroupViewModel> groups;
    int ownIndex;
};


#endif //VIEWMODELS_H
