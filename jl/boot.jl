Base.load_juliarc()
include("Jlts.jl")
cfunction(Jlts.process, Void, (Ptr{Ptr{Void}},))
