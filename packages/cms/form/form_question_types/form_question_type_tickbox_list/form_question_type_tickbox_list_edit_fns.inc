<?php

require_once dirname(__FILE__).'/../../form_question/form_question_edit_fns.inc';

/**
* Form_Question_Type_Tickbox_List_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Form_Question_Type_Tickbox_List_Edit_Fns extends Form_Question_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Form_Question_Type_Tickbox_List_Edit_Fns()
	{
		$this->Form_Question_Edit_Fns();
	}

	function paintDefault(&$asset, &$o, $prefix)
	{
		$options = explode("\r\n", $asset->attr('options'));
		$multiple = ($asset->attr('multiple')) ? true : false;
		
		echo "<table cellpadding=\"0\" cellspacing=\"0\">\n";
		
		foreach ($options as $ref => $option) {
			if (!trim($option)) continue;

			echo '<tr><td>';
			$val  = explode(":", $option);
			$type = ($multiple) ? 'check_box' : 'radio_button';
			$name = 'default['.$ref.']'; 
			
			$title = (count($val) > 1) ? $val[1] : $val[0];
			$d = $asset->attr('default');
			$checked = isset($d[$ref]) ? true : false;

			$type($name, $ref, $checked);
			echo '&nbsp;'.$title;

			echo '</td></tr>';
		}
		echo '</table>';

		return true;
	}

	function processDefault(&$asset, &$o, $prefix)
	{
		if (isset($_POST['default'])) {
			bam($_POST['default']);
			if (!$asset->setAttrValue('default', $_POST['default'])) {
				return false;
			}
		// if nothing has been submitted, then nothing has been selected, so blank all defaults out
		} else {
			$asset->setAttrValue('default', Array());
		}
		return true;
	}


}//end class
?>
