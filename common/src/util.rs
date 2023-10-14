use std::array::TryFromSliceError;

pub fn slice_to_array<T, const N: usize>(value: &[T]) -> Result<&[T; N], TryFromSliceError> {
    value.try_into()
}
