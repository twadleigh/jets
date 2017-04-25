Base.load_juliarc()
println("Running boot file...")
include("Jlts.jl")
async = Jlts.create_worker(Jlts.sum_floats)
yield()
println(async)
async.handle
