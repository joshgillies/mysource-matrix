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
* $Id: option_list.inc,v 1.14 2005/03/31 22:42:29 lwright Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';

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
* @version $Revision: 1.14 $
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
			$i = 0;

			if (!empty($this->value)) {

				foreach ($current_value as $option) {
					$current_to_string .= '<div id="'.$prefix.'_'.$i.'">[ <a href="#" onclick="Javascript: optionListRemoveOption('.$i.', \''.$prefix.'\', \'options\'); return false;">del</a> ] <span id="'.$prefix.'_content_'.$i.'">'.htmlspecialchars($option).'</span></div>';
					$current_to .= $option."\r\n";
					$i++;
				}

			}

		?>

		<script language="Javascript">
			// need to define 'next index' stuff as an array in case there is more than one option
			// list/email format attribute on the screen
			if (typeof next_index == 'undefined') {
				next_index = new Array();
			}
			next_index['<?php echo $prefix ?>'] = <?php echo $i ?>;

			function optionListAddOption(box_name, type) {
				var value = document.getElementById(box_name + "_" + type);
				var box = document.getElementById(box_name + "_new_" + type);
				var div = document.getElementById(box_name + "_current_" + type);

				if (box.value == "") return;

				div.innerHTML += '<div id="' + box_name + '_' + next_index[box_name].toString() + '">[ <a href="#" onclick="Javascript: optionListRemoveOption(' + next_index[box_name].toString() + ', \'' + box_name + '\', \'' + type + '\'); return false;">del</a> ] <span id="' + box_name + '_content_' + next_index[box_name].toString() + '">' + box.value.replace('<', '&lt;').replace('>', '&gt;') + '</span></div>';
				value.value   += box.value + "\r\n";

				next_index[box_name]++;

				box.value = "";
				box.focus();

			}//end optionListAddOption()


			function optionListRemoveOption(index, box_name, type) {
				var div = document.getElementById(box_name + "_current_" + type);
				var value = document.getElementById(box_name + "_" + type);

				arrIndex = 0;
				option_array = new Array();

				// get the first div in the list
				divX = div.firstChild;

				while(divX != null) {
					// already hidden?
					if (divX.style.display != 'none') {
						this_option = divX.firstChild.nextSibling.nextSibling.nextSibling.innerHTML;

						// this is the one we want
						if (divX.id == (box_name + "_" + index)) {
							divX.style.display = 'none';
						} else {
							option_array[arrIndex] = this_option.replace('&lt;', '<').replace('&gt;', '>');
						}
						arrIndex++;
					}
					divX = divX.nextSibling;
				}

				var new_value_content = '';

				for (var i = 0; i < option_array.length; i++) {
					var option = option_array[i];
					if (typeof option != 'undefined') {
						re = new RegExp("\\\[.*\\\] (.*)", "g");
						this_option = option.replace(re, "\$1");

						new_value_content += this_option + "\r\n";
					}
				}

				value.value = new_value_content;


			}//end optionListRemoveOption()

		</script>
			<?php text_box($prefix.'_new_options', '', 50); ?>
			[ <a href="#" onclick="optionListAddOption('<?php echo $prefix ?>', 'options'); return false;">add</a> ]<br/>
			<div id="<?php echo $prefix; ?>_current_options"><?php echo $current_to_string; ?></div>
			<span style="display:none"><?php text_area($prefix.'_options', $current_to, 50, 5); ?></span>
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

		if (!isset($_REQUEST[$prefix.'_options'])) {
			return false;
		}

		$value = Array();

		$to = Array();
		if (isset($_REQUEST[$prefix.'_options'])) {
			$to = explode("\r\n", trim($_REQUEST[$prefix.'_options']));
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