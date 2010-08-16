<?php
// $Id

class geoclustering_handler_join_dummy extends views_join
{
	function join($table,&$query)
		{
			return "";		// Generate no SQL for this join
			}
	}
