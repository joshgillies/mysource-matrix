<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: option_list.inc,v 1.8 2004/09/08 04:17:48 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


/**
* Option List Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <option_list />
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Option_List extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Option_List($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, $value);

	}//end constructor


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		$prefix = str_replace(':','_',$prefix); // handle shadow assets

		$current_value = explode("\r\n", $this->value);
		if (!is_array($current_value))             $current_value = Array();
		
		if ($read_only) {
			echo implode('<br />', $current_value);
		} else {	
			$current_to_string = '';
			$current_to = '';
			if (!empty($this->value)) {
				foreach ($current_value as $option) {
					$current_to_string .= '[ <a href="#" onclick="Javascript: optionListRemoveOption(\''.$option.'\', \''.$prefix.'\', \'options\'); return false;">del</a> ] '.$option.'<br>';
					$current_to .= $option."\r\n";
				}
			}

		?>
		<script language="Javascript">

			function optionListAddOption(box_name, type) {
				var value = document.getElementById(box_name + "_" + type);
				var box = document.getElementById(box_name + "_new_" + type);
				var div = document.getElementById(box_name + "_current_" + type);
				
				if (box.value == "") return;

				div.innerHTML += '[ <a href="#" onclick="Javascript: optionListRemoveOption(\'' + box.value + '\', \'' + box_name + '\', \'' + type + '\'); return false;">del</a> ] ' + box.value + '<br>';
				value.value += box.value + "\r\n";
				
				box.value = "";
				
			}//end optionListAddOption()


			function optionListRemoveOption(opt_delete, box_name, type) {
				var value = document.getElementById(box_name + "_" + type);
				var div = document.getElementById(box_name + "_current_" + type);
				
				var current_options = div.innerHTML;

				lastIndex = i = 0;
				arrIndex = 0;
				option_array = new Array();
				while(-1 != (i = current_options.toLowerCase().indexOf("<br>", lastIndex))) {
					option_array[arrIndex] = current_options.substring(lastIndex, i);
					arrIndex++;
					lastIndex = i+4;
				}

				var new_div_content = '';
				var new_value_content = '';

				for (var i = 0; i < option_array.length; i++) {
					var option = option_array[i];
					re = new RegExp("\\\[.*\\\] (.*)", "gi");
					this_option = option.replace(re, "\$1");

					if (this_option != opt_delete && this_option != "") {
						new_div_content += option + "<br>";
						new_value_content += this_option + "\r\n";
					}
				}
				
				div.innerHTML = new_div_content;
				value.value = new_value_content;

			}//end optionListRemoveOption()
		</script>
			<?php text_box($prefix.'_new_options', '', 50); ?>
			[ <a href="#" onclick="Javascript: optionListAddOption('<?php echo $prefix ?>', 'options'); return false;">add</a> ]<br/>
			<div id="<?php echo $prefix; ?>_current_options"><?php echo $current_to_string; ?></div>
			<span style="display:none;"><?php text_area($prefix.'_options', $current_to, 5, 1); ?></span>
		<?php
		}

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	prefix	prefix for the form elements
	*
	* @access public
	*/
	function process($prefix)
	{
		$prefix = str_replace(':','_',$prefix);	// handle shadow assets

		if (!isset($_POST[$prefix.'_options'])) {
			return false;
		}

		$value = Array();

		$to = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$to = explode("\r\n", trim($_POST[$prefix.'_options']));
			// make sure the emails are valid
			foreach ($to as $option) {
				$option = trim($option);
				if (empty($option)) continue;
				$value[] = $option;
			}
		}
		
		$this->processed = $this->setValue(implode("\r\n",$value));

	}//end process()


}//end class

?>