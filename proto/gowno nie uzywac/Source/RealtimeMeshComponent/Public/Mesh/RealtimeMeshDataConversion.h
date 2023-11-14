﻿// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "RealtimeMeshDataTypes.h"

namespace RealtimeMesh
{
	using FRealtimeMeshElementDataConverter = TFunction<void(const void*, void*)>;
	using FRealtimeMeshContiguousElementDataConverter = TFunction<void(const void*, void*, uint32)>;

	
	struct REALTIMEMESHCOMPONENT_API FRealtimeMeshElementConversionKey
	{
		const FRealtimeMeshElementType FromType;
		const FRealtimeMeshElementType ToType;

		FRealtimeMeshElementConversionKey(const FRealtimeMeshElementType& InFromType, const FRealtimeMeshElementType& InToType)
			: FromType(InFromType), ToType(InToType)
		{
		}

		bool operator==(const FRealtimeMeshElementConversionKey& OtherKey) const
		{
			return FromType == OtherKey.FromType && ToType == OtherKey.ToType;
		}

		friend uint32 GetTypeHash(const FRealtimeMeshElementConversionKey& Key)
		{
			return HashCombine(GetTypeHash(Key.FromType), GetTypeHash(Key.ToType));
		}
	};

	struct REALTIMEMESHCOMPONENT_API FRealtimeMeshElementConverters
	{
	private:
		const FRealtimeMeshElementDataConverter ElementConverter;
		const FRealtimeMeshContiguousElementDataConverter ContiguousArrayConverter;

	public:
		FRealtimeMeshElementConverters(const FRealtimeMeshElementDataConverter& InElementConverter,
		                               const FRealtimeMeshContiguousElementDataConverter& InContiguousArrayConverter)
			: ElementConverter(InElementConverter)
			, ContiguousArrayConverter(InContiguousArrayConverter)
		{
		}

		void ConvertSingleElement(const void* Input, void* Output) const
		{
			ElementConverter(Input, Output);
		}
		void ConvertContiguousArray(const void* Input, void* Output, uint32 NumElements) const
		{
			ContiguousArrayConverter(Input, Output, NumElements);
		}
	};

	struct REALTIMEMESHCOMPONENT_API FRealtimeMeshTypeConversionUtilities
	{
	private:
		static TMap<FRealtimeMeshElementConversionKey, FRealtimeMeshElementConverters> TypeConversionMap;

	public:
		static bool CanConvert(const FRealtimeMeshElementType& FromType, const FRealtimeMeshElementType& ToType);
		static const FRealtimeMeshElementConverters& GetTypeConverter(const FRealtimeMeshElementType& FromType, const FRealtimeMeshElementType& ToType);
		static void RegisterTypeConverter(const FRealtimeMeshElementType& FromType, const FRealtimeMeshElementType& ToType,
		                                  const FRealtimeMeshElementConverters& Converters);
		static void UnregisterTypeConverter(const FRealtimeMeshElementType& FromType, const FRealtimeMeshElementType& ToType);
	};

	template <typename FromType, typename ToType>
	class FRealtimeMeshTypeConverterRegistration : FNoncopyable
	{
		static_assert(FRealtimeMeshElementTypeTraits<FromType>::IsValid);
		static_assert(FRealtimeMeshElementTypeTraits<ToType>::IsValid);

	public:
		FRealtimeMeshTypeConverterRegistration(FRealtimeMeshElementConverters Converters)
		{
			FRealtimeMeshTypeConversionUtilities::RegisterTypeConverter(GetRealtimeMeshDataElementType<FromType>(), GetRealtimeMeshDataElementType<ToType>(), Converters);
		}

		~FRealtimeMeshTypeConverterRegistration()
		{
			FRealtimeMeshTypeConversionUtilities::UnregisterTypeConverter(GetRealtimeMeshDataElementType<FromType>(), GetRealtimeMeshDataElementType<ToType>());
		}
	};


#define RMC_DEFINE_ELEMENT_TYPE_CONVERTER(FromElementType, ToElementType, ElementConverter) \
	FRealtimeMeshTypeConverterRegistration<FromElementType, ToElementType> GRegister##FromElementType##To##ToElementType(FRealtimeMeshElementConverters( \
			[](const void* SourceElement, void* DestinationElement) { \
				const FromElementType& Source = *static_cast<const FromElementType*>(SourceElement); \
				ToElementType& Destination = *static_cast<ToElementType*>(DestinationElement); \
				ElementConverter \
			}, \
			[](const void* SourceArr, void* DestinationArr, uint32 Count) { \
				const FromElementType* SourceArrT = static_cast<const FromElementType*>(SourceArr); \
				ToElementType* DestinationArrT = static_cast<ToElementType*>(DestinationArr); \
				for (uint32 Index = 0; Index < Count; Index++) \
				{ \
					const FromElementType& Source = SourceArrT[Index]; \
					ToElementType& Destination = DestinationArrT[Index]; \
					ElementConverter; \
				} \
			} \
		) \
	);

#define RMC_DEFINE_ELEMENT_TYPE_CONVERTER_TRIVIAL(FromElementType, ToElementType) \
	RMC_DEFINE_ELEMENT_TYPE_CONVERTER(FromElementType, ToElementType, { Destination = ToElementType(Source); });
}
