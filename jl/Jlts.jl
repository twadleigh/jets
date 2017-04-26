module Jlts

function process(wi :: Ptr{Ptr{Void}})

    # lock the work item mutex
    mtx = unsafe_load(wi, 1)
    ccall(:uv_mutex_lock, Void, (Ptr{Void},), mtx)

    # do something with the io
    io = unsafe_load(wi, 3)
    fptr = convert(Ptr{Float64}, io)
    unsafe_store!(fptr, 42.0)

    # set done to be non-null
    unsafe_store!(wi, C_NULL-1, 4)

    # signal condition
    cnd = unsafe_load(wi, 2)
    ccall(:uv_cond_signal, Void, (Ptr{Void},), cnd)

    # free the work item mutex
    ccall(:uv_mutex_unlock, Void, (Ptr{Void},), mtx)
end

end
