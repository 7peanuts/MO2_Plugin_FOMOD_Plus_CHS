﻿#include "FomodViewModel.h"
#include "xml/ModuleConfiguration.h"

/*
--------------------------------------------------------------------------------
                               Lifecycle
--------------------------------------------------------------------------------
*/
/**
 * 
 * @param organizer
 * @param fomodFile 
 * @param infoFile 
 */
FomodViewModel::FomodViewModel(MOBase::IOrganizer *organizer,
                               std::unique_ptr<ModuleConfiguration> fomodFile,
                               std::unique_ptr<FomodInfoFile> infoFile)
: mOrganizer(organizer), mFomodFile(std::move(fomodFile)), mInfoFile(std::move(infoFile)), mConditionTester(organizer),
  mInfoViewModel(std::move(infoFile)) {
}

/**
 *
 * @param organizer
 * @param fomodFile
 * @param infoFile
 * @return
 */
std::shared_ptr<FomodViewModel> FomodViewModel::create(MOBase::IOrganizer *organizer,
                                                       std::unique_ptr<ModuleConfiguration> fomodFile,
                                                       std::unique_ptr<FomodInfoFile> infoFile) {
  auto viewModel = std::make_shared<FomodViewModel>(organizer, std::move(fomodFile), std::move(infoFile));
  viewModel->createStepViewModels();
  viewModel->mActiveStep = viewModel->mSteps.at(0);
  viewModel->mActivePlugin = viewModel->getFirstPluginForActiveStep();
  std::cout << "Active Plugin: " << viewModel->mActivePlugin->getName() << std::endl;
  return viewModel;
}

/*
--------------------------------------------------------------------------------
                               Initialization
--------------------------------------------------------------------------------
*/

void FomodViewModel::collectFlags() {
  if (mSteps.empty()) {
    return;
  }
  for (const auto step : mSteps) {
    for (auto flagDependency : step->installStep->visible.dependencies.flagDependencies) {
      mFlags.setFlag(flagDependency.flag, "");
    }
  }
}

void FomodViewModel::constructInitialStates() {
  // For each group, "select" the correct plugin based on the spec.
  for (const auto step : mSteps) {
    for (const auto group : step->getGroups()) {
      switch (group->getType()) {
        case SelectExactlyOne:
          // Mark the first option that doesn't fail its condition as active
          break;
        case SelectAtLeastOne:
          break;
        case SelectAll:
          // set every plugin in this group to be checked and disabled
            for (auto plugin : group->getPlugins()) {
              togglePlugin(group, plugin, true);
              plugin->setEnabled(false);
            }
          break;
        // SelectAny, SelectAtMostOne, and don't need anything to be done
        default: ;
      }
    }
  }
}

void FomodViewModel::createStepViewModels() {
  shared_ptr_list<StepViewModel> stepViewModels;

  for (const auto& installStep : mFomodFile->installSteps.installSteps) {
    std::vector<std::shared_ptr<GroupViewModel>> groupViewModels;

    for (const auto& group : installStep.optionalFileGroups.groups) {
      std::vector<std::shared_ptr<PluginViewModel>> pluginViewModels;

      for (const auto& plugin : group.plugins.plugins) {
        auto pluginViewModel = std::make_shared<PluginViewModel>(std::make_shared<Plugin>(plugin), false, true);
        pluginViewModels.emplace_back(pluginViewModel); // Assuming default values for selected and enabled
      }
      auto groupViewModel = std::make_shared<GroupViewModel>(std::make_shared<Group>(group), std::move(pluginViewModels));
      groupViewModels.emplace_back(groupViewModel);
    }
    auto stepViewModel = std::make_shared<StepViewModel>(std::make_shared<InstallStep>(installStep), std::move(groupViewModels));
    stepViewModels.emplace_back(stepViewModel);
  }
  // TODO Sort the view models here, maybe
  collectFlags();
  updateVisibleSteps();
  mSteps = std::move(stepViewModels);
}


std::shared_ptr<PluginViewModel> FomodViewModel::getFirstPluginForActiveStep() const {
  if (mSteps.empty()) {
    throw std::runtime_error("No steps found in FomodViewModel");
  }
  return mActiveStep->getGroups().at(0)->getPlugins().at(0);
}


// onpluginselected should also take a group option to set the values for the other plugins, possibly
// TODO: Handle groups later
void FomodViewModel::togglePlugin(std::shared_ptr<GroupViewModel>, const std::shared_ptr<PluginViewModel> &plugin, const bool selected) {
  plugin->setSelected(selected);
  for (auto flag : plugin->getPlugin()->conditionFlags.flags) {
    if (selected) {
      mFlags.setFlag(flag.name, flag.value);
    } else {
      mFlags.setFlag(flag.name, "");
    }
  }
  updateVisibleSteps();
}


/*
--------------------------------------------------------------------------------
                               Step Visibility
--------------------------------------------------------------------------------
*/
bool FomodViewModel::isStepVisible(const int stepIndex) const {
  const auto step = mSteps[stepIndex]->installStep;
  return mConditionTester.isStepVisible(mFlags, step.get());
}

void FomodViewModel::updateVisibleSteps() {
  mVisibleStepIndices.clear();
  for (int i = 0; i < mSteps.size(); ++i) {
    if (isStepVisible(i)) {
      mVisibleStepIndices.push_back(i);
    }
  }
}

bool FomodViewModel::isLastVisibleStep() const {
  return !mVisibleStepIndices.empty() && mCurrentStepIndex == mVisibleStepIndices.back();
}

/*
--------------------------------------------------------------------------------
                               Navigation
--------------------------------------------------------------------------------
*/
void FomodViewModel::stepBack() {
  const auto it = std::ranges::find(mVisibleStepIndices, mCurrentStepIndex);
  if (it != mVisibleStepIndices.end() && it != mVisibleStepIndices.begin()) {
    mCurrentStepIndex = *std::prev(it);
    mActiveStep = mSteps[mCurrentStepIndex];
  }
}

void FomodViewModel::stepForward() {
  const auto it = std::ranges::find(mVisibleStepIndices, mCurrentStepIndex);
  if (it != mVisibleStepIndices.end() && std::next(it) != mVisibleStepIndices.end()) {
    mCurrentStepIndex = *std::next(it);
    mActiveStep = mSteps[mCurrentStepIndex];
  }
}

/*
--------------------------------------------------------------------------------
                               Flags
--------------------------------------------------------------------------------
*/
void FomodViewModel::setFlag(const std::string &flag, const std::string &value) {
  mFlags.setFlag(flag, value);
}

std::string FomodViewModel::getFlag(const std::string &flag) {
  return mFlags.getFlag(flag);
}

