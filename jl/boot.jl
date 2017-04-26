Base.load_juliarc()
include("Jets.jl")
cfunction(Jets.process, Void, (Ptr{Ptr{Void}},))
