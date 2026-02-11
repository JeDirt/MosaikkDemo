// Fill out your copyright notice in the Description page of Project Settings.

#include "MSequencer/MosaikkTrack.h"

#include "MCore/MosaikkTypes.h"
#include "MSequencer/MosaikkSection.h"

#define LOCTEXT_NAMESPACE "SequenceAnnotationTrack"

UMosaikkTrack::UMosaikkTrack()
{
	
}

bool UMosaikkTrack::IsEmpty() const
{
	return Sections.IsEmpty();
}

void UMosaikkTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.AddUnique(&Section);
}

void UMosaikkTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}

void UMosaikkTrack::RemoveSectionAt(int32 SectionIndex)
{
	Sections.RemoveAt(SectionIndex);
}

bool UMosaikkTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMosaikkSection::StaticClass();
}

UMovieSceneSection* UMosaikkTrack::CreateNewSection()
{
	return NewObject<UMosaikkSection>(this, NAME_None, RF_Transactional);
}

const TArray<UMovieSceneSection*>& UMosaikkTrack::GetAllSections() const
{
	return Sections;
}

bool UMosaikkTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}

bool UMosaikkTrack::SupportsMultipleRows() const
{
	return false;
}

EMovieSceneTrackEasingSupportFlags UMosaikkTrack::SupportsEasing(FMovieSceneSupportsEasingParams& Params) const
{
	return EMovieSceneTrackEasingSupportFlags::None;
}

void UMosaikkTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}

#if WITH_EDITORONLY_DATA
FText UMosaikkTrack::GetDefaultDisplayName() const
{
	return LOCTEXT("DisplayName", "Sequence Annotation Track");
}

void UMosaikkTrack::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITORONLY_DATA
	const UMosaikkSettings* MosaikkSettingsCDO = GetDefault<UMosaikkSettings>();
	TrackTint = MosaikkSettingsCDO->DefaultTrackTint;
#endif
}
#endif
#undef LOCTEXT_NAMESPACE
