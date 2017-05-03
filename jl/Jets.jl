module Jets

# processes work item
function process(handler, wi :: Ptr{Ptr{Void}})
    # lock the work item mutex
    mtx = unsafe_load(wi, 1)
    ccall(:uv_mutex_lock, Void, (Ptr{Void},), mtx)

    # do something with the io
    io = unsafe_load(wi, 3)
    try
        handler(io)
    catch e
        showerror(STDERR, e)
    end

    # set done to be non-null
    unsafe_store!(wi, C_NULL-1, 4)

    # signal condition
    cnd = unsafe_load(wi, 2)
    ccall(:uv_cond_signal, Void, (Ptr{Void},), cnd)

    # free the work item mutex
    ccall(:uv_mutex_unlock, Void, (Ptr{Void},), mtx)

    nothing
end

function wrap_handler(handler::Function)
    function _submit(wi :: Ptr{Ptr{Void}}) :: Void
        @schedule process(handler, wi)
        yield()
        nothing
    end
    cfunction(_submit, Void, (Ptr{Ptr{Void}},))
end

end
