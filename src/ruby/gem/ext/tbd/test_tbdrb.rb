# Tests for Tbdrb module.

require 'test/unit'
require 'tbdrb'




class TbdrbTestCase  < Test::Unit::TestCase

	def test_create_read_update_delete
	
	    tbd = Class.new.extend(Tbdrb)
	
	    
		result = tbd.create("key", "value")
		
        assert_equal(0, result, "create error")
		
		
		result = tbd.read("key")
		
		assert_equal("value", result, "read error")
		
		
		result = tbd.update("key", "VALUE")
		
		assert_equal(0, result, "update error")
		
	    
		result = tbd.read("key")
	
		assert_equal("VALUE", result)
		
	
		result = tbd.delete("key")
		
		assert_equal(0, result, "delete error")
    end

end