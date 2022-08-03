#include "context-package/ReasonerContext.h"

#include <pluginlib/class_list_macros.h>

#include "ontologenius/graphical/Display.h"

namespace ontologenius
{
  void ReasonerContext::initialize()
  {
    getContextProperties();
  }

  void ReasonerContext::postReason()
  {
    // if (context_properties_.size() == 0)
    //   getContextProperties();

    std::lock_guard<std::shared_timed_mutex> lock(ontology_->individual_graph_.mutex_);
    std::vector<IndividualBranch_t *> indivs = ontology_->individual_graph_.get();
    for (auto &indiv : indivs)
    {
      if (indiv->updated_ == true)
      {
        // Display::info("--ReasonerContext-- Updated individual :" + indiv->value());
        auto get_up_set = ontology_->individual_graph_.getUp(indiv);
        if (get_up_set.find("Agent") != get_up_set.end())
        {
          Display::info("Agent updated : " + indiv->value());
          // debug(indiv);
          std::unordered_set<IndividualBranch_t *> better_agent_ctx = getBetterContextByAgent(indiv);
          size_t size_valid_agent_ctx = better_agent_ctx.size();
          Display::info("Size of better ctx " + std::to_string(size_valid_agent_ctx));
          if (size_valid_agent_ctx != 0)
          {
            if (size_valid_agent_ctx == 1)
            {
              Display::info("Only 1 ctx valid");
              // Display::info("Id of current ctx : " + std::to_string(getCurrentCtxAgent(indiv)->get()) + " value : " + getCurrentCtxAgent(indiv)->value());
              // Display::info("Id of btter ctx found : " + std::to_string((*better_agent_ctx.begin())->get()) + " value : " + (*better_agent_ctx.begin())->value());

              if (getCurrentCtxAgent(indiv)->get() == (*better_agent_ctx.begin())->get())
              {
                Display::success("Current ctx is the best !! : " + getCurrentCtxAgent(indiv)->value());
              }
              else
              {
                Display::info("Set to the only one valid ctx");
                setCurrentAgentCtx(indiv, (*better_agent_ctx.begin()));
              }
            }
            else
            {
              IndividualBranch_t *more_precise_ctx = filterBetterCtx(better_agent_ctx, indiv);
              setCurrentAgentCtx(indiv, more_precise_ctx);
            }
          }
          else
          {
            Display::warning("No Context valid for agent");
            Display::info("Create ctx");
            createCtxForAgent(indiv);
          }
        }
      }
    }

    Display::info("nb_updates : " + std::to_string(nb_update_));

    Display::info("--------------------------------------------------------");
    Display::info("");
    Display::info("");
    Display::info("");
    // Display::info("");
    // Display::info("");
    // Display::info("");
    // Display::info("");
    // Display::info("");
    // Display::info("");
    // Display::info("");
  }

  void ReasonerContext::createCtxForAgentWithActivity(IndividualBranch_t *indiv)
  {
    Display::warning("CreateWith activity : " + indiv->value());
  }

  bool ReasonerContext::activityMatch(IndividualBranch_t *agent, IndividualBranch_t *ctx)
  {
    Display::info("Check Activity : " + agent->value() + " ctx : " + ctx->value());
    IndividualBranch_t *agent_activity = getActivityOfAgent(agent);
    if (agent_activity == nullptr)
    {
      Display::info("Agent without activity");
      return true;
    }
    else
    {
      std::unordered_set<IndividualBranch_t *> activity_ctx = getActivityOfCtx(ctx);

      // return activity_ctx.size() ? activity_ctx.find(agent_activity) !=activity_ctx.end() : true;

      if (activity_ctx.size() == 0)
      {
        Display::info("Ctx without activity");
        return true;
      }
      else
      {
        return activity_ctx.find(agent_activity) != activity_ctx.end();
      }
    }
  }

  void ReasonerContext::debug(IndividualBranch_t *agent)
  {
    Display::info("Agent : " + agent->value());
    for (auto &relation : agent->object_relations_)
    {
      Display::info("    ----> relation : " + relation.first->value() + ": object pointed : " + relation.second->value());
    }
  }

  void ReasonerContext::setCurrentAgentCtx(IndividualBranch_t *agent, IndividualBranch_t *ctx)
  {

    auto old_ctx = getCurrentCtxAgent(agent);
    Display::info("Remove old current ctx : " + old_ctx->value() + " for nex ctx : " + ctx->value());
    if (old_ctx->get() != ctx->get())
    {
      // Display::info("agent : " + agent->value() + " ctx : " + old_ctx->value());
      ontology_->individual_graph_.removeRelation(agent, prop_current_ctx_, old_ctx);
      if (ctxEmptyAfterRemoveAgent(old_ctx, agent))
      {
        Display::info("ctx is empty and with mark as finish");
        ontology_->individual_graph_.addRelation(old_ctx, data_is_finish, data_t("boolean", "true"), 1, true);
      }
      old_ctx->nb_updates_++;
      // Display::info("Set current ctx");
      Display::success("agent : " + agent->value() + " ctx : " + ctx->value());
      ontology_->individual_graph_.addRelation(agent, prop_current_ctx_, ctx, 1.0, true);
      agent->nb_updates_++;
      ctx->nb_updates_++;
      nb_update_++;
    }
    else
    {
      // Display::info("Ctx already set!");
      Display::success("Current ctx is the best !! : " + ctx->value());
    }
  }

  IndividualBranch_t *ReasonerContext::filterBetterCtx(const std::unordered_set<IndividualBranch_t *> &better_ctx, IndividualBranch_t *agent)
  {
    std::unordered_set<uint32_t> exclude_ctx_;
    for (auto ctx : better_ctx)
    {
      for (auto ctx2 : better_ctx)
      {
        if (ctx->get() != ctx2->get())
        {
          if (exclude_ctx_.find(ctx->get()) == exclude_ctx_.end() and exclude_ctx_.find(ctx2->get()) == exclude_ctx_.end())
          {
            if (ctxInclude(ctx, ctx2))
            {
              exclude_ctx_.insert(ctx->get());
            }
          }
        }
      }
    }
    std::unordered_set<IndividualBranch_t *> keep_ctx_;
    for (auto ctx : better_ctx)
    {
      if (exclude_ctx_.find(ctx->get()) == exclude_ctx_.end())
      {
        Display::info("ctx keep : " + ctx->value());
        keep_ctx_.insert(ctx);
      }
    }

    Display::info("Nombre de context encore valable : " + std::to_string(keep_ctx_.size()));
    if (keep_ctx_.size() == 0)
    {
      Display::error("Plus aucun context valable !!!");
      return nullptr;
    }
    else
    {
      Display::info("Better filter ctx : " + (*keep_ctx_.begin())->value());
      return *(keep_ctx_.begin());
    }
  }

  bool ReasonerContext::ctxInclude(IndividualBranch_t *ctx, IndividualBranch_t *ctx2)
  {
    // ObjectPropertyBranch_t *prop_has_in = ontology_->object_property_graph_.findBranch("hasIn");
    auto loc_ctx = getLocationOfCtx(ctx);
    auto loc_ctx2 = getLocationOfCtx(ctx2);
    if (loc_ctx != nullptr and loc_ctx2 != nullptr)
    {
      return locInclude(loc_ctx, loc_ctx2);
    }
    return false;
  }

  std::unordered_set<IndividualBranch_t *> ReasonerContext::getLocationsOfAgent(IndividualBranch_t *agent)
  {
    ObjectPropertyBranch_t *prop_is_in = ontology_->object_property_graph_.findBranch("isIn");
    std::unordered_set<IndividualBranch_t *> list_loc_;
    for (auto &relation : agent->object_relations_)
    {
      std::unordered_set<uint32_t> list_getUp_prop;
      ontology_->object_property_graph_.getUpIdSafe(relation.first, list_getUp_prop);
      if (list_getUp_prop.find(prop_is_in->get()) != list_getUp_prop.end())
      {
        // Display::info("Propriete trouve et la valeur de l'individu pointé est: " + relation.second->value());
        list_loc_.insert(relation.second);
      }
    }
    return list_loc_;
  }

  IndividualBranch_t *ReasonerContext::morePreciseLocation(std::unordered_set<IndividualBranch_t *> list_loc)
  {
    if (list_loc.size() > 0)
    {
      if (list_loc.size() == 1)
      {
        return *list_loc.begin();
      }
      std::unordered_set<uint32_t> list_loc_exclude_;

      for (auto loc : list_loc)
      {
        for (auto loc2 : list_loc)
        {
          if (loc->get() != loc2->get())
          {
            if (list_loc_exclude_.find(loc->get()) == list_loc_exclude_.end() and list_loc_exclude_.find(loc2->get()) == list_loc_exclude_.end())
            {
              if (locInclude(loc, loc2))
              {
                list_loc_exclude_.insert(loc->get());
              }
            }
          }
        }
      }
      std::unordered_set<IndividualBranch_t *> keep_loc_;
      for (auto loc : list_loc)
      {
        if (list_loc_exclude_.find(loc->get()) == list_loc_exclude_.end())
        {
          Display::info("loc keep : " + loc->value());
          keep_loc_.insert(loc);
        }
      }
      if (keep_loc_.size() == 0)
      {
        Display::error("Plus aucune localisation valable !!!");
        return nullptr;
      }
      else
      {
        Display::info("Better filter loc : " + (*keep_loc_.begin())->value());
        return *(keep_loc_.begin());
      }
    }
    return nullptr;
  }

  IndividualBranch_t *ReasonerContext::getLocationOfCtx(IndividualBranch_t *ctx)
  {
    for (auto &relation : ctx->object_relations_)
    {
      if (relation.first->get() == prop_is_compose_of_env->get())
      {
        auto env_ctx = relation.second;
        for (auto &env_relation : env_ctx->object_relations_)
        {
          if (env_relation.first->get() == prop_is_env_ctx_take_place_->get())
          {
            return env_relation.second;
          }
        }
      }
    }
    return nullptr;
  }

  std::unordered_set<IndividualBranch_t *> ReasonerContext::getBetterContextByAgent(IndividualBranch_t *indiv)
  {
    std::unordered_set<IndividualBranch_t *> agent_ctx = getContextByAgent(indiv);
    std::unordered_set<IndividualBranch_t *> res = {};
    int max = 1;
    for (auto ctx : agent_ctx)
    {
      int score = validateCtxAgent(ctx, indiv);

      Display::info("ctx : " + ctx->value() + " score : " + std::to_string(score));
      if (score == max)
      {
        res.insert(ctx);
      }
      if (score > max)
      {
        max = score;
        res.clear();
        res.insert(ctx);
      }
    }
    Display::info("Result better ctx for this agent : " + indiv->value());
    for (auto ctx : res)
    {
      Display::info("   - " + ctx->value());
    }
    return res;
  }

  std::string ReasonerContext::getName()
  {
    return "reasoner context";
  }

  std::string ReasonerContext::getDesciption()
  {
    return "This is an reasoner model to show how to create your own reasoner plugin";
  }

  bool ReasonerContext::ctxEmptyAfterRemoveAgent(IndividualBranch_t *ctx, IndividualBranch_t *agent)
  {
    Display::info("ctx empty " + ctx->value() + " ?");
    for (auto &relation : ctx->object_relations_)
    {
      if (is_compose_of_list_.find(relation.first->get()) != is_compose_of_list_.end())
      {
        // Display::info("sub ctx find "+relation.second->value());
        for (auto &rel : relation.second->object_relations_)
        {
          // Display::info("Inv : "+prop_is_in_env->inverses_[0].elem->value());
          if (rel.first->get() == prop_is_in_env->inverses_[0].elem->get())
          {
            // Display::info("Inv found : "+prop_is_in_env->inverses_[0].elem->value());
            Display::info("Inv found : " + rel.second->value());
            if (rel.second->get() != agent->get())
            {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

  IndividualBranch_t *ReasonerContext::getActivityOfAgent(IndividualBranch_t *agent)
  {
    for (auto &relation : agent->object_relations_)
    {
      if (relation.first->get() == prop_agent_do_activity->get())
      {
        Display::info("Activity of agent : " + relation.second->value());
        return relation.second;
      }
    }
    return nullptr;
  }

  std::unordered_set<IndividualBranch_t *> ReasonerContext::getActivityOfCtx(IndividualBranch_t *ctx)
  {
    // Display::info("-------"+prop_agent_do_activity->inverses_[0].elem->value());
    Display::info("get activity of ctx : " + ctx->value());
    std::unordered_set<IndividualBranch_t *> res;
    for (auto &relation : ctx->object_relations_)
    {

      // Display::info("relation : " + relation.first->value());
      if (relation.first->get() == prop_is_compose_of_act->get())
      {
        auto activity_ctx = relation.second;
        // Display::info("activity ctx : " + activity_ctx->value());
        for (auto &relation_activity_ctx : relation.second->object_relations_)
        {

          // Display::info("relation_activity_ctx.first : " + relation_activity_ctx.first->value());
          if (relation_activity_ctx.first->get() == prop_activity_ctx_has_activity->get())
          {
            // auto activity  = relation_activity_ctx.second;
            // Display::info("Activity : "+relation_activity_ctx.second->value());
            res.insert(relation_activity_ctx.second);
          }
        }
      }
    }
    // Display::info("Activity found in the ctx : " + ctx->value());
    // for (auto act : res)
    // {
    //   Display::info("   -> " + act->value());
    // }
    return res;
  }

  std::unordered_set<IndividualBranch_t *> ReasonerContext::getContextByAgent(IndividualBranch_t *indiv)
  {
    std::unordered_set<IndividualBranch_t *> res;

    for (auto &relation : indiv->object_relations_)
    {
      if (relation.first->get() == prop_is_in_env->get())
      {
        // Display::info("relation is in env : "+relation.second->value());
        for (auto &rel : relation.second->object_relations_)
        {
          if (rel.first->get() == prop_is_part_of_->get())
          {

            // Display::info("-------------> TEST data properties on : " + rel.second->value());
            auto data = std::find_if(rel.second->data_relations_.begin(), rel.second->data_relations_.end(), [this](IndivDataRelationElement_t dataRelation)
                                     { return dataRelation.first->get() == data_is_finish->get(); });
            if (data != std::end(rel.second->data_relations_))
            {
              if (data->second.value_ == "false")
              {
                res.insert(rel.second);
              }
              else
              {
                // Display::info("data : " + data->first->value());
                Display::info("This ctx is finish : " + rel.second->value());
              }
            }
            else
            {
              res.insert(rel.second);
            }
          }
        }
      }
    }

    // Display::info("ctx found : ");
    // for (auto ctx : res)
    // {
    //   Display::info(" -- " + ctx->value());
    // }
    return res;
  }

  IndividualBranch_t *ReasonerContext::getCurrentCtxAgent(IndividualBranch_t *agent)
  {
    for (auto &relation : agent->object_relations_)
    {
      if (relation.first->get() == prop_current_ctx_->get())
      {
        return relation.second;
      }
    }
    return nullptr;
  }

  void ReasonerContext::createCtxForAgent(IndividualBranch_t *agent)
  {

    if (getCurrentCtxAgent(agent) == nullptr)
    {
      int id = generateIdForNewCtx();
      std::map<std::string, IndividualBranch_t *> ctxs = createCtx(id);
      ontology_->individual_graph_.addRelation(agent, prop_current_ctx_, ctxs["global"], 1.0, true);
      setEnvLocationByAgent(ctxs["env"], agent);
      // ontology_->individual_graph_.addRelation(agent, prop_is_in_env, ctxs.second, 1.0, true);
      for (auto &prop : context_properties_)
      {
        std::unordered_set<IndividualBranch_t *> agent_values_by_prop_id = getAgentPropertyIdValues(agent, prop.second);
        if (agent_values_by_prop_id.size() != 0)
        {
          Display::info("Agent with ctx link values : ");
          for (auto val : agent_values_by_prop_id)
          {
            Display::info(" --->  " + val->value());
          }
        }
      }
    }
    else
    {
      if (!activityMatch(agent, getCurrentCtxAgent(agent)))
      {
        ontology_->individual_graph_.removeRelation(agent, prop_current_ctx_, getCurrentCtxAgent(agent));
        Display::info("Create With Act");
        int id = generateIdForNewCtx();
        std::map<std::string, IndividualBranch_t *> ctxs = createCtx(id);
        ontology_->individual_graph_.addRelation(agent, prop_current_ctx_, ctxs["global"], 1.0, true);
        setEnvLocationByAgent(ctxs["env"], agent);
        setActivityByAgent(ctxs["activity"], agent);
        // ontology_->individual_graph_.addRelation(agent, prop_is_in_env, ctxs.second, 1.0, true);
        // for (auto &prop : context_properties_)
        // {
        //   std::unordered_set<IndividualBranch_t *> agent_values_by_prop_id = getAgentPropertyIdValues(agent, prop.second);
        //   if (agent_values_by_prop_id.size() != 0)
        //   {
        //     Display::info("Agent with ctx link values : ");
        //     for (auto val : agent_values_by_prop_id)
        //     {
        //       Display::info(" --->  " + val->value());
        //     }
        //   }
        // }
      }
    }
  }

  void ReasonerContext::setEnvLocationByAgent(IndividualBranch_t *envCtx, IndividualBranch_t *agent)
  {

    std::unordered_set<IndividualBranch_t *> list_loc_ = getLocationsOfAgent(agent);
    IndividualBranch_t *bestLocation = morePreciseLocation(list_loc_);
    if (bestLocation != nullptr)
    {
      ontology_->individual_graph_.addRelation(envCtx, prop_is_env_ctx_take_place_, bestLocation, 1.0, true);
    }
  }

  void ReasonerContext::setActivityByAgent(IndividualBranch_t *ctx, IndividualBranch_t *agent)
  {

    IndividualBranch_t *actAgent = getActivityOfAgent(agent);
    Display::info("Activity : " + actAgent->value() + " ctx : " + ctx->value());
    ontology_->individual_graph_.addRelation(ctx, prop_activity_ctx_has_activity, actAgent, 1.0, true);
  }

  bool ReasonerContext::locInclude(IndividualBranch_t *loc, IndividualBranch_t *loc2)
  {
    ObjectPropertyBranch_t *prop_has_in = ontology_->object_property_graph_.findBranch("hasIn");
    for (auto &relation : loc->object_relations_)
    {
      std::unordered_set<uint32_t> list_getUp_prop;
      ontology_->object_property_graph_.getUpIdSafe(relation.first, list_getUp_prop);
      if (list_getUp_prop.find(prop_has_in->get()) != list_getUp_prop.end())
      {
        // Display::info("ctx include : " + relation.second->value() + " loc of ctx 2 : " + loc_ctx2->value());
        if (relation.second->get() == loc2->get())
        {
          Display::info(loc2->value() + " is include in " + loc->value());
          return true;
        }
      }
    }
    return false;
  }

  std::map<std::string, IndividualBranch_t *> ReasonerContext::createCtx(int id)
  {
    std::map<std::string, IndividualBranch_t *> res;
    IndividualBranch_t *global_ctx = ontology_->individual_graph_.createIndividualUnsafe("globalCtx_" + std::to_string(id));
    IndividualBranch_t *env_ctx = ontology_->individual_graph_.createIndividualUnsafe("environmentCtx_" + std::to_string(id));
    IndividualBranch_t *act_ctx = ontology_->individual_graph_.createIndividualUnsafe("activityCtx_" + std::to_string(id));
    IndividualBranch_t *social_ctx = ontology_->individual_graph_.createIndividualUnsafe("socialCtx_" + std::to_string(id));
    res.insert({"global", global_ctx});
    res.insert({"env", env_ctx});
    res.insert({"activity", act_ctx});
    res.insert({"social", social_ctx});
    ontology_->individual_graph_.addInheritageUnsafe(env_ctx, "EnvironnementalContext");
    ontology_->individual_graph_.addInheritageUnsafe(global_ctx, "GlobalContext");
    ontology_->individual_graph_.addInheritageUnsafe(act_ctx, "ActivityContext");
    ontology_->individual_graph_.addInheritageUnsafe(social_ctx, "SocialContext");

    ontology_->individual_graph_.addRelation(global_ctx, prop_is_compose_of_env, env_ctx);
    ontology_->individual_graph_.addRelation(global_ctx, prop_is_compose_of_act, act_ctx);
    ontology_->individual_graph_.addRelation(global_ctx, prop_is_compose_of_social, social_ctx);

    return res;
    // ontology_->individual_graph_.addRelation(env_ctx,prop_id_is_part_of_,global_ctx);
  }

  int ReasonerContext::generateIdForNewCtx()
  {
    int iter = 1;
    std::srand(time(NULL));
    int id = std::rand() % (int)(std::pow(10, iter));
    Display::info("id generated : " + std::to_string(id));
    while (ontology_->individual_graph_.findBranchUnsafe("globalCtx_" + std::to_string(id)) != nullptr)
    {
      iter++;
      id = std::rand() % (int)std::pow(10, iter);
      Display::info("Not null id found new id : " + std::to_string(id));
    }
    Display::info("final id generated : " + std::to_string(id));
    return id;
  }

  int ReasonerContext::validateCtxAgent(IndividualBranch_t *ctx, IndividualBranch_t *indiv)
  {
    // Display::info("Validate agent : " + indiv->value() + " to context : " + ctx->value());
    int valid_prop_count = 0;
    for (auto &prop : context_properties_)
    {
      bool valid_prop = false;
      std::unordered_set<IndividualBranch_t *> agent_values_by_prop_id = getAgentPropertyIdValues(indiv, prop.second);
      for (auto value : agent_values_by_prop_id)
      {
        for (auto &relation : value->object_relations_)
        {
          if (relation.first->get() == prop.first)
          {
            // Display::info("Relation tested : " + relation.first->value());
            uint32_t id = getContextIdOfSubContext(relation.second);
            if (id == (uint32_t)-1)
            {
              Display::error("subcontext not link to global context");
            }
            if (id == ctx->get())
            {
              // Display::info("Valid id et ctx id : " + relation.first->value() + " " + relation.second->value());
              valid_prop = true;
              valid_prop_count++;
              break;
            }
            else
            {
              // Display::info("Invalid id ctx : " + relation.first->value() + " " + relation.second->value());
            }
          }
        }
        // valid_prop = true;
        if (valid_prop)
        {
          // Display::info("Break valid indiv found");
          break;
        }
      }
      // Display::info("score : " + std::to_string(valid_prop_count));
      // if (!valid_prop)
      // {
      //   Display::info("Invalid prop return false : " + std::to_string(prop.first) + " " + indiv->value());

      //   return 0;
      // }
    }

    return activityMatch(indiv, ctx) ? valid_prop_count : 0;
  }

  uint32_t ReasonerContext::getContextIdOfSubContext(IndividualBranch_t *sub_ctx)
  {
    for (auto &relation : sub_ctx->object_relations_)
    {
      if (relation.first->get() == prop_is_part_of_->get())
      {
        return relation.second->get();
      }
    }
    return (uint32_t)-1;
  }

  std::unordered_set<IndividualBranch_t *> ReasonerContext::getAgentPropertyIdValues(IndividualBranch_t *agent, uint32_t id_prop)
  {
    std::unordered_set<IndividualBranch_t *> res;
    for (auto &relation : agent->object_relations_)
    {
      // Display::info("Prop : " + std::to_string(relation.first->get()) + " name : " + relation.first->value());
      std::unordered_set<uint32_t> list_getUp_prop;
      ontology_->object_property_graph_.getUpIdSafe(relation.first, list_getUp_prop);
      if (list_getUp_prop.find(id_prop) != list_getUp_prop.end())
      {
        // Display::info("Propriete trouve et la valeur de l'individu pointé est: " + relation.second->value());
        res.insert(relation.second);
      }
    }

    // Display::info("indiv found : ");
    // for (auto indiv : res)
    // {
    //   Display::info(" -- " + indiv->value());
    // }

    return res;
  }

  void ReasonerContext::getContextProperties()
  {
    std::map<std::string, std::string> properties_str = {{"locationsupportEnvironnementLocation", "isIn"},
                                                         {"RoleperformInSocialContext", "agentPlayRole"},
                                                         {"activityPerformInActivityContext", "agentIsUseInActivity"}};

    for (auto &property_str : properties_str)
    {
      auto prop_ctx = ontology_->object_property_graph_.findBranch(property_str.first);
      auto prop_agent = ontology_->object_property_graph_.findBranch(property_str.second);

      if ((prop_ctx != nullptr) && (prop_agent != nullptr))
      {
        context_properties_.insert({prop_ctx->get(), prop_agent->get()});
        Display::info("prop ctx: " + property_str.first + " value : " + std::to_string(prop_ctx->get()));
        Display::info("prop agent: " + property_str.second + " value : " + std::to_string(prop_agent->get()));
      }
    }

    ObjectPropertyBranch_t *global_context_is_compose_of_context_prop = ontology_->object_property_graph_.findBranch("globalContextIsComposeOfContext");
    ontology_->object_property_graph_.getDownIdSafe(global_context_is_compose_of_context_prop, is_compose_of_list_);
    prop_is_part_of_ = ontology_->object_property_graph_.findBranch("contextIsPartOfGlobalContext");

    prop_current_ctx_ = ontology_->object_property_graph_.findBranch("agentCurrentContext");

    prop_is_in_env = ontology_->object_property_graph_.findBranch("agentIsInEnvironnementContext");
    prop_is_compose_of_env = ontology_->object_property_graph_.findBranch("globalContextIsComposeOfEnvironnementalContext");
    prop_is_compose_of_social = ontology_->object_property_graph_.findBranch("globalContextIsComposeOfEnvironnementalContext");
    prop_is_compose_of_act = ontology_->object_property_graph_.findBranch("globalContextIsComposeOfActivityContext");

    prop_is_env_ctx_take_place_ = ontology_->object_property_graph_.findBranch("environnementContextTakePlaceInLocation");
    prop_agent_do_activity = ontology_->object_property_graph_.findBranch("agentIsUseInActivity");
    prop_activity_ctx_has_activity = ontology_->object_property_graph_.findBranch("activityContextHasActivity");
    data_is_finish = ontology_->data_property_graph_.findBranch("isFinish");
  }

  PLUGINLIB_EXPORT_CLASS(ReasonerContext, ReasonerInterface)

} // namespace ontologenius
