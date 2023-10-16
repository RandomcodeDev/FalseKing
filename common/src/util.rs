use std::array::TryFromSliceError;

pub fn slice_to_array<T, const N: usize>(value: &[T]) -> Result<&[T; N], TryFromSliceError> {
    value.try_into()
}

/// From https://stackoverflow.com/questions/28127165/how-to-convert-struct-to-u8
pub unsafe fn any_as_bytes<T: Sized>(p: &T) -> &[u8] {
    ::core::slice::from_raw_parts(
        (p as *const T) as *const u8,
        ::core::mem::size_of::<T>(),
    )
}
