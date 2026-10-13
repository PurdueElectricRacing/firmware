//! Private, strict wire models. Required nullable fields must be present.

use serde::Deserialize as _;
use std::{fmt, marker};

include!(concat!(env!("OUT_DIR"), "/superdbc_models.rs"));

pub use MessageExport as MessageModel;
pub use SignalExport as SignalModel;
pub use SuperDbcExport as Document;

pub fn narrow<T: TryInto<U>, U>(
    value: T,
    context: &str,
    field: &str,
) -> Result<U, super::error::ParseError> {
    value.try_into().map_err(|_| {
        super::error::ParseError::definition(
            context,
            format!("{field} is outside its supported numeric range"),
        )
    })
}

#[derive(serde::Deserialize)]
pub struct VersionProbe {
    pub versions: SchemaProbe,
}

#[derive(serde::Deserialize)]
pub struct SchemaProbe {
    pub schema_version: String,
}

#[derive(serde::Deserialize)]
struct UniqueEntries<K: Eq + std::hash::Hash + fmt::Debug, T>(
    #[serde(deserialize_with = "unique_map")]
    #[serde(bound(deserialize = "K: serde::Deserialize<'de>, T: serde::Deserialize<'de>"))]
    indexmap::IndexMap<K, T>,
);

fn nullable_unique_map<'de, D, K, T>(
    deserializer: D,
) -> Result<Option<indexmap::IndexMap<K, T>>, D::Error>
where
    D: serde::Deserializer<'de>,
    K: serde::Deserialize<'de> + Eq + std::hash::Hash + fmt::Debug,
    T: serde::Deserialize<'de>,
{
    Ok(Option::<UniqueEntries<K, T>>::deserialize(deserializer)?.map(|entries| entries.0))
}

fn required_nullable<'de, D, T>(deserializer: D) -> Result<Option<T>, D::Error>
where
    D: serde::Deserializer<'de>,
    T: serde::Deserialize<'de>,
{
    // Using deserialize_with without a default makes omission an error, while
    // still allowing an explicit JSON null to deserialize as None.
    Option::deserialize(deserializer)
}

// A normal map deserializer silently overwrites repeated JSON property names.
// Reject them instead, while preserving document order without serde_json::Value.
fn unique_map<'de, D, K, T>(deserializer: D) -> Result<indexmap::IndexMap<K, T>, D::Error>
where
    D: serde::Deserializer<'de>,
    K: serde::Deserialize<'de> + Eq + std::hash::Hash + fmt::Debug,
    T: serde::Deserialize<'de>,
{
    struct UniqueMap<K, T>(marker::PhantomData<(K, T)>);
    impl<'de, K, T> serde::de::Visitor<'de> for UniqueMap<K, T>
    where
        K: serde::Deserialize<'de> + Eq + std::hash::Hash + fmt::Debug,
        T: serde::Deserialize<'de>,
    {
        type Value = indexmap::IndexMap<K, T>;
        fn expecting(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
            f.write_str("an object with unique keys")
        }
        fn visit_map<A: serde::de::MapAccess<'de>>(
            self,
            mut map: A,
        ) -> Result<Self::Value, A::Error> {
            let mut result = indexmap::IndexMap::new();
            // Consume the original object entries before a normal map could
            // silently replace repeated keys and hide ambiguous definitions.
            while let Some((key, value)) = map.next_entry::<K, T>()? {
                if result.contains_key(&key) {
                    return Err(serde::de::Error::custom(format!("duplicate key {key:?}")));
                }

                result.insert(key, value);
            }

            Ok(result)
        }
    }

    deserializer.deserialize_map(UniqueMap(marker::PhantomData))
}
