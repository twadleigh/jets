module Jlts

const live_dict = ObjectIdDict() # dictionary of values for which references exist on the C side

const libuv = Libdl.dlopen("libuv-1")
const uv_cond_signal = Libdl.dlsym(libuv, :uv_cond_signal)

# structure passed to worker
struct WorkerData
    io :: Ptr{Void}
    cond :: Ptr{Void} # points to uv_cond_t
    done :: Ptr{Void} # exists only to distinguish from spurious wake-up
end

# tell caller to stop waiting
function notify_done(wd :: Ptr{WorkerData})
    ptr = convert(Ptr{Void}, wd) # treat as pointer to void pointers
    unsafe_store!(ptr, C_NULL-1, 3) # set done to non-null pointer value
    cond = unsafe_load(ptr, 2) # get pointer to condition variable
    ccall(uv_cond_signal, Void, (Ptr{Void},), cond) # signal condition variable for wakeup
end

# creates a worker task executing the given function
function create_worker(body :: Function)

    # create an async condition variable by which the worker will be notified of work to be done
    async = AsyncCondition()

    # start the worker loop
    @async begin
        try
            while true
                # wait for a request
                Base.wait(async)

                # get the WorkerData (which we assume has been passed on the C side)
                ptr = Base.uv_handle_data(async)

                # get the io pointer
                io = unsafe_load(ptr, 1)

                # invoke the body with the io pointer
                body(io)

                # notify the caller that we are done
                notify_done(convert(Ptr{WorkerData}, wd))
            end
        catch
            # any exception that escapes the call to body causes the worker to die
            rethrow()
        end
    end

    # return the condition on which the worker waits
    return async
end

function sum_floats(io :: Ptr{Void})
    # get values to add
    ptr = convert(Ptr{Float64}, io)
    a = unsafe_load(ptr, 1)
    b = unsafe_load(ptr, 2)
    unsafe_store!(ptr, a + b, 3) # put sum in third slot
end

end
