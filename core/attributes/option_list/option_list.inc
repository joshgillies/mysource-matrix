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
* $Id: option_list.inc,v 1.1 2004/05/19 05:55:14 lwright Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_ATTRIBUTES_PATH.'/serialise/serialise.inc';

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
	function Asset_Attribute_Email_Format($attributeid=0, $value=null)
	{
		$this->Asset_Attribute_Serialise($attributeid, $value);

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
		$prefix = $prefix.'_'.$this->name;
		$current_value = explode("\r\n", $this->value);
		if (!is_array($current_value))             $current_value = Array();
		
		if ($read_only) {
			echo implode('<br />', $current_value);
		} else {	
			$current_to_string = '';
			$current_to = '';
			foreach ($current_value as $email) {
				$current_to_string .= '[ <a href="#" onclick="Javascript: optionListRemoveOption(\''.$email.'\', \'options\'); return false;">del</a> ] '.$email.'<br>';
				$current_to .= $email."\r\n";
			}

		?>
		<script language="Javascript">

			function optionListAddOption(type) {
				var value = document.getElementById("<?php echo $prefix; ?>_" + type);
				var box = document.getElementById("<?php echo $prefix; ?>_new_" + type);
				var div = document.getElementById("<?php echo $prefix; ?>_current_" + type);
				
				if (box.value == "") return;

				div.innerHTML += '[ <a href="#" onclick="Javascript: optionListRemoveOption(\'' + box.value + '\', \'' + type + '\'); return false;">del</a> ] ' + box.value + '<br>';
				value.value += box.value + "\r\n";
				
				box.value = "";
				
			}//end emailFormatAddAddress()


			function optionListRemoveOption(address, type) {
				alert("<?php echo $prefix; ?>_" + type);
				var value = document.getElementById("<?php echo $prefix; ?>_" + type);
				var div = document.getElementById("<?php echo $prefix; ?>_current_" + type);
				
				var current_emails = div.innerHTML;

				lastIndex = i = 0;
				arrIndex = 0;
				email_array = new Array();
				while(-1 != (i = current_emails.toLowerCase().indexOf("<br>", lastIndex))) {
					email_array[arrIndex] = current_emails.substring(lastIndex, i);
					arrIndex++;
					lastIndex = i+4;
				}
				alert(email_array);

				var new_div_content = '';
				var new_value_content = '';

				for (var i = 0; i < email_array.length; i++) {
					var email = email_array[i];
					re = new RegExp("\\\[.*\\\] (.*)", "gi");
					email_addr = email.replace(re, "\$1");
					alert(email_addr);

					if (email_addr != address && email_addr != "") {
						new_div_content += email + "<br>";
						new_value_content += email_addr + "\r\n";
					}
				}
				
				div.innerHTML = new_div_content;
				value.value = new_value_content;

			}//end emailFormatDeleteAddress()
		</script>
			<?php text_box($prefix.'_new_options', '', 50); ?>
			[ <a href="#" onclick="Javascript: optionListAddOption('options'); return false;">add</a> ]<br/>
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
		$prefix = $prefix.'_'.$this->name;

		if (!isset($_POST[$prefix.'_options'])) {
			return false;
		}

		$value = Array();

		$to = Array();
		if (isset($_POST[$prefix.'_options'])) {
			$to = explode("\r\n", trim($_POST[$prefix.'_options']));
			// make sure the emails are valid
			foreach ($to as $email) {
				$email = trim($email);
				if (empty($email)) continue;
				$value[] = $email;
			}
		}
		
		$this->processed = $this->setValue(implode("\r\n",$value));

	}//end process()


	/**
	* Ensure the attribute being set is an array
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	/*function validateValue(&$value)
	{
		// cant serilise scalar variables
		if (is_scalar($value)) {
			// see if it is already serilised
			$unser = @unserialize(trim((string) $value));
			if (is_scalar($unser)) return false;
			else $value = $unser;
		}
		
		if (!is_array($value)) return false;

		$value = serialize($value);
		return true;

	}*///end validateValue()


}//end class

?>