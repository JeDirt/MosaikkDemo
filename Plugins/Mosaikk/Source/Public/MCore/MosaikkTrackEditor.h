#pragma once

#include "MovieSceneTrackEditor.h"

class FMenuBuilder;

class FMosaikkTrackEditor : public FMovieSceneTrackEditor
{
public:
	FMosaikkTrackEditor(TSharedRef<ISequencer> InSequencer);

	/**
	 * Factory function to create an instance of this class (called by a sequencer).
	 *
	 * @param InSequencer The sequencer instance to be used by this tool.
	 * @return The new instance of this class.
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);

public:
	// Begin FMovieSceneTrackEditor interface
	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	// ~End FMovieSceneTrackEditor interface

protected:
	void AddMosaikkTrackSubMenuRoot(FMenuBuilder& MenuBuilder, TArray<FGuid>);

	void AddTrackToSequence(const FAssetData& InAssetData);
	void AddTrackToSequenceEnterPressed(const TArray<FAssetData>& InAssetData);
};