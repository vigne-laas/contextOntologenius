#ifndef CONTEXTPACKAGE_REASONERCONTEXT_H
#define CONTEXTPACKAGE_REASONERCONTEXT_H

#include "ontologenius/core/reasoner/plugins/ReasonerInterface.h"

namespace ontologenius
{

  class ReasonerContext : public ReasonerInterface
  {
  public:
    ReasonerContext() {}
    virtual ~ReasonerContext() = default;

    virtual void postReason() override;
    virtual void initialize() override;

    virtual std::string getName() override;
    virtual std::string getDesciption() override;

  private:
    std::map<uint32_t, uint32_t> context_properties_;
    std::unordered_set<uint32_t> is_compose_of_list_;

    ObjectPropertyBranch_t *prop_is_part_of_;
    ObjectPropertyBranch_t *prop_current_ctx_;
    ObjectPropertyBranch_t *prop_is_in_env;
    ObjectPropertyBranch_t *prop_is_compose_of_env;
    ObjectPropertyBranch_t *prop_is_env_ctx_take_place_;
    ObjectPropertyBranch_t *prop_agent_do_activity;
    ObjectPropertyBranch_t *prop_activity_ctx_has_activity;
    ObjectPropertyBranch_t *prop_is_compose_of_social;
    ObjectPropertyBranch_t *prop_is_compose_of_act;
    DataPropertyBranch_t *data_is_finish;

    // std::unordered_set<IndividualBranch_t *> all_ctx;

    std::unordered_set<IndividualBranch_t *> getContextByAgent(IndividualBranch_t *indiv);
    std::unordered_set<IndividualBranch_t *> getBetterContextByAgent(IndividualBranch_t *indiv);

    void getContextProperties();
    void createCtxForAgent(IndividualBranch_t *agent);
    int generateIdForNewCtx();

    bool activityMatch(IndividualBranch_t *indiv, IndividualBranch_t *ctx);
    IndividualBranch_t *getActivityOfAgent(IndividualBranch_t *agent);
    std::unordered_set<IndividualBranch_t *> getActivityOfCtx(IndividualBranch_t *ctx);

    std::map<std::string, IndividualBranch_t *> createCtx(int id);
    IndividualBranch_t *getLocationOfCtx(IndividualBranch_t *ctx);
    std::unordered_set<IndividualBranch_t *> getLocationsOfAgent(IndividualBranch_t *agent);
    void setEnvLocationByAgent(IndividualBranch_t *ctx, IndividualBranch_t *agent);
    void setActivityByAgent(IndividualBranch_t *ctx, IndividualBranch_t *agent);
    IndividualBranch_t *morePreciseLocation(std::unordered_set<IndividualBranch_t *> list_loc);

    void debug(IndividualBranch_t *agent);

    void createCtxForAgentWithActivity(IndividualBranch_t *indiv);

    bool ctxInclude(IndividualBranch_t *ctx, IndividualBranch_t *ctx2);
    bool locInclude(IndividualBranch_t *loc, IndividualBranch_t *loc2);

    bool ctxEmptyAfterRemoveAgent(IndividualBranch_t *ctx, IndividualBranch_t *agent);
    void setCurrentAgentCtx(IndividualBranch_t *agent, IndividualBranch_t *ctx);
    IndividualBranch_t *filterBetterCtx(const std::unordered_set<IndividualBranch_t *> &better_ctx, IndividualBranch_t *agent);

    IndividualBranch_t *getCurrentCtxAgent(IndividualBranch_t *agent);
    uint32_t getContextIdOfSubContext(IndividualBranch_t *sub_ctx);
    std::unordered_set<IndividualBranch_t *> getAgentPropertyIdValues(IndividualBranch_t *agent, uint32_t id_prop);
    int validateCtxAgent(IndividualBranch_t *ctx, IndividualBranch_t *indiv);
  };

} // namespace ontologenius

#endif // CONTEXTPACKAGE_REASONERCONTEXT_H
