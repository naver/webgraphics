#ifndef PLAYBACKMANAGER_H
#define PLAYBACKMANAGER_H

#include "2D.h"
#include "CommandList.h"
#include "Filters.h"
#include "RecordedEvent.h"

#include <vector>
#ifdef __GNUC__
#include <ext/hash_set>
using __gnu_cxx::hash_set;
namespace __gnu_cxx {
#define DEFINE_TRIVIAL_HASH(integral_type) \
    template<> \
    struct hash<integral_type> { \
      std::size_t operator()(integral_type value) const { \
        return (std::size_t)(value); \
      } \
    }
    DEFINE_TRIVIAL_HASH(void*);
}
#else
#include <hash_set>
using std::hash_set;
#endif
#include <map>

class PlaybackManager : public mozilla::gfx::Translator
{
public:
    PlaybackManager();
    ~PlaybackManager();

    typedef mozilla::gfx::DrawTarget DrawTarget;
    typedef mozilla::gfx::Path Path;
    typedef mozilla::gfx::SourceSurface SourceSurface;
    typedef mozilla::gfx::FilterNode FilterNode;
    typedef mozilla::gfx::GradientStops GradientStops;
    typedef mozilla::gfx::ScaledFont ScaledFont;
    typedef mozilla::gfx::NativeFontResource NativeFontResource;

    virtual DrawTarget *LookupDrawTarget(mozilla::gfx::ReferencePtr aRefPtr);
    virtual Path *LookupPath(mozilla::gfx::ReferencePtr aRefPtr);
    virtual SourceSurface *LookupSourceSurface(mozilla::gfx::ReferencePtr aRefPtr);
    virtual FilterNode *LookupFilterNode(mozilla::gfx::ReferencePtr aRefPtr);
    virtual GradientStops *LookupGradientStops(mozilla::gfx::ReferencePtr aRefPtr);
    virtual ScaledFont *LookupScaledFont(mozilla::gfx::ReferencePtr aRefPtr);
    virtual mozilla::gfx::NativeFontResource *LookupNativeFontResource(uint64_t aKey);
    virtual DrawTarget *GetReferenceDrawTarget() { return mBaseDT; }
    virtual mozilla::gfx::FontType GetDesiredFontType();
    virtual void AddDrawTarget(mozilla::gfx::ReferencePtr aRefPtr, DrawTarget *aDT) { mDrawTargets[aRefPtr] = aDT; }
    virtual void RemoveDrawTarget(mozilla::gfx::ReferencePtr aRefPtr) { mDrawTargets.erase(aRefPtr); }
    virtual void AddPath(mozilla::gfx::ReferencePtr aRefPtr, Path *aPath) { mPaths[aRefPtr] = aPath; }
    virtual void AddSourceSurface(mozilla::gfx::ReferencePtr aRefPtr, SourceSurface *aSurface) { mSourceSurfaces[aRefPtr] = aSurface; }
    virtual void RemoveSourceSurface(mozilla::gfx::ReferencePtr aRefPtr) { mSourceSurfaces.erase(aRefPtr); }
    virtual void RemovePath(mozilla::gfx::ReferencePtr aRefPtr) { mPaths.erase(aRefPtr); }
    virtual void AddGradientStops(mozilla::gfx::ReferencePtr aRefPtr, GradientStops *aStops) { mGradientStops[aRefPtr] = aStops; }
    virtual void RemoveGradientStops(mozilla::gfx::ReferencePtr aRefPtr) { mGradientStops.erase(aRefPtr); }
    virtual void AddScaledFont(mozilla::gfx::ReferencePtr aRefPtr, ScaledFont *aFont) { mScaledFonts[aRefPtr] = aFont; }
    virtual void AddNativeFontResource(uint64_t, mozilla::gfx::NativeFontResource *);
    virtual void RemoveScaledFont(mozilla::gfx::ReferencePtr aRefPtr) { mScaledFonts.erase(aRefPtr); }
    virtual void AddFilterNode(mozilla::gfx::ReferencePtr aRefPtr, FilterNode *aNode) { mFilterNodes[aRefPtr] = aNode; }
    virtual void RemoveFilterNode(mozilla::gfx::ReferencePtr aRefPtr) { mFilterNodes.erase(aRefPtr); }


    void SetBaseDT(DrawTarget *aBaseDT) { mBaseDT = aBaseDT; }
    void AddEvent(mozilla::gfx::RecordedEvent *aEvent) { mRecordedEvents.push_back(aEvent); }
    const mozilla::gfx::RecordedEvent * GetEvent(uint32_t aID) const { return mRecordedEvents[aID]; }

    void PlaybackToEvent(int aID);

    void DisableEvent(uint32_t aID);
    void EnableEvent(uint32_t aID);
    void EnableAllEvents();
    bool IsEventDisabled(uint32_t aID);
    double GetEventTiming(uint32_t aID, bool aAllowBatching, bool aIgnoreFirst, bool aDoFlush, bool aForceCompletion, double *aStdDev);

    uint32_t GetCurrentEvent() { return mCurrentEvent; }

    size_t GetEventSize() const { return mRecordedEvents.size(); }

    void SetCommandListCtrl(CCommandList* commandListCtrl) { mCommandListCtrl = commandListCtrl; }
    CCommandList& GetCommandListCtrl() { return *mCommandListCtrl; }

    typedef std::map<void*, RefPtr<DrawTarget>> DTMap;
    typedef std::map<void*, RefPtr<Path>> PathMap;
    typedef std::map<void*, RefPtr<SourceSurface>> SourceSurfaceMap;
    typedef std::map<void*, RefPtr<GradientStops>> GradientStopsMap;
    typedef std::map<void*, RefPtr<ScaledFont>> ScaledFontMap;
    typedef std::map<uint64_t, RefPtr<NativeFontResource>> NativeFontResourceMap;
    typedef std::map<void*, RefPtr<FilterNode>> FilterNodeMap;

    DTMap mDrawTargets;
    PathMap mPaths;
    SourceSurfaceMap mSourceSurfaces;
    GradientStopsMap mGradientStops;
    ScaledFontMap mScaledFonts;
    NativeFontResourceMap mNativeFontResources;
    FilterNodeMap mFilterNodes;
    std::vector<mozilla::gfx::RecordedEvent*> mRecordedEvents;
    hash_set<uint32_t> mDisabledEvents;
    void EventDisablingUpdated(int32_t aID);

private:
    friend class PlaybackTranslator;

    bool IsClipPush(uint32_t aID, int32_t aRefID = -1);
    bool IsClipPop(uint32_t aID, int32_t aRefID = -1);
    bool FindCorrespondingClipID(uint32_t aID, uint32_t *aOtherID);

    void PlayToEvent(uint32_t aID);
    void PlaybackEvent(mozilla::gfx::RecordedEvent *aEvent);

    bool CanDisableEvent(mozilla::gfx::RecordedEvent *aEvent);
    bool DisableOneTimeEvent(uint32_t aID);

    void ForceCompletion();

    uint32_t mCurrentEvent;
    uint32_t mEventSize;
    CCommandList* mCommandListCtrl;

    RefPtr<mozilla::gfx::DrawTarget> mBaseDT;
};

#endif // PLAYBACKMANAGER_H
