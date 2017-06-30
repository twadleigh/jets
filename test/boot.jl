Base.load_juliarc()
include(joinpath(Base.source_dir(), "..", "jl", "Jets.jl"))

# handle work item payload
function handle(io :: Ptr{Void})
    fptr = convert(Ptr{Float64}, io)
    unsafe_store!(fptr, 42.0)
end

# creates the C-function pointer for handling work items
Jets.wrap_handler(handle)
