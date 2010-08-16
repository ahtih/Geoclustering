<?php
// $Id

class geoclustering_handler_field_summed_fields extends
												views_handler_field_numeric
{
	function init(&$view,$options)
		{
			parent::init($view,$options);

			$this_view_summed_fields=array('volume','trustworthiness');	//!!! load from ctools/DB

			$flipped=array_flip($this_view_summed_fields);
			$this->summed_fields_idx=$flipped[
							substr($this->field,strlen('summed_field_'))];
			}

	function render($values)
		{
			//!!! replace this with SQL query() handler
			$summed_fields=explode(' ',$values->{$this->field_alias});
			return (float)$summed_fields[$this->summed_fields_idx];
			}
	}
