# Tests for Tbdrb module.

require 'benchmark'
require 'tbdrb'




Benchmark.bm do|b|
	tbd = Class.new.extend(Tbdrb)
	
	b.report("Tbdrb") do
		1_000_000.times { 
		  tbd.create("key", "value")
		  result = tbd.read("key")
		  tbd.update("key", "VALUE")
		  result = tbd.read("key")
		  tbd.delete("key")
		}
	end

	b.report("Hash") do
		1_000_000.times { 
		   a = Hash.new()
		   a["key"] = "value"
		   result = a["key"]
		   a["key"] = "VALUE"
		   result = a["key"]
		   a["key"] = nil
		}
	end

end