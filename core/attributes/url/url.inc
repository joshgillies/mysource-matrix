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
* $Id: url.inc,v 1.1 2005/06/21 00:33:54 ndvries Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';

/**
* URL Attribute
*
* <!-- Sample Edit Interface XML Node -->
*    <text width="40" protocols="http,https,ftp,mailto" />
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Url extends Asset_Attribute
{

	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Url($attributeid=0, $value=null)
	{
		$this->Asset_Attribute($attributeid, $value);

		// set default edit parameters
		$this->_edit_params['width']  = 0;
		$this->_edit_params['protocols'] = Array('http', 'https');

	}//end constructor


	/**
	* Print the interface for changing edit params
	*
	* @param string		$prefix		prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function paintEditParams($prefix, $write_access=false)
	{
		echo '<b>URL Attribute " '.ucwords(str_replace('_', ' ', $this->name)).' " ('.$this->name.')</b><br />';
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('width'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_'.$this->name.'_width', $this->_edit_params['width']);
					else echo $this->_edit_params['width'];
					?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-section-heading" style="width: 100px;"><?php echo translate('protocols'); ?></td>
				<td class="sq-backend-table-cell">
					<?php
					if ($write_access) text_box($prefix.'_'.$this->name.'_height', implode(',', $this->_edit_params['protocols']));
					else echo $this->_edit_params['protocols'];
					?>
				</td>
			</tr>
		</table>
		<?php

	}//end paintEditParams()


	/**
	* Process the interface for changing edit params
	*
	* @param string		$prefix		prefix for the form elements
	*
	* @return array
	* @access public
	*/
	function processEditParams($prefix)
	{
		// reading submitted vars
		if (!empty($_POST[$prefix.'_'.$this->name.'_width'])) {
			$this->_edit_params['width'] = $_POST[$prefix.'_'.$this->name.'_width'];
		}

		if (!empty($_POST[$prefix.'_'.$this->name.'_protocols'])) {
			$protocols = str_replace(' ', '', $_POST[$prefix.'_'.$this->name.'_protocols']);
			$this->_edit_params['protocols'] = explode(',', $protocols);
		}

		$values = Array();
		$values['width']  = $this->_edit_params['width'];
		$values['protocols'] = $this->_edit_params['protocols'];

		return $values;

	}//end processEditParams()


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		if (isset($node->attributes['width']))  $this->_edit_params['width']  = (int) $node->attributes['width'];
		if (isset($node->attributes['protocols'])) {
			$protocols = str_replace(' ', '', $node->attributes['protocols']);
			$this->_edit_params['protocols'] = explode(',', $protocols);
		}

		return true;

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		if ($read_only) {
			echo htmlspecialchars(nl2br($this->value));
			return;
		}

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		?><script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/JsHttpConnector.js"></script><?php
		?><script type="text/javascript">

			function getFullUrl() {
				var protocol = document.getElementById("<?php echo $prefix.'_ccheck_protocol_dropdown'; ?>").value;
				var url = document.getElementById("<?php echo $prefix.'_ccheck_input_box'; ?>").value;
				var fullUrl = protocol + url;

				return fullUrl;
			}

			function changeStatusText(responseState)
			{
				switch (responseState) {
					case "0":
						responseText = js_translate('non_existant_page');
					break;
					case "1":
						responseText = js_translate('valid_page');
					break;
					default:
						responseText = js_translate('unknown_state');
				}

				document.getElementById('<?php echo $prefix.'_ccheck_status_text'; ?>').innerHTML = responseText;

			}//end changeStatusText()

			function checkUrlExists()
			{
				document.getElementById('<?php echo $prefix.'_ccheck_status_text'; ?>').innerHTML = js_translate('testing');
				var testUrl = "<?php echo sq_web_path('lib').'/web/connectivity.php'; ?>" + "?connect_url=" + getFullUrl();
				JsHttpConnector.submitRequest(testUrl, changeStatusText);

			}//end checkUrlExists()


			function protocolChanged()
			{
				var protocol = document.getElementById('<?php echo $prefix.'_ccheck_protocol_dropdown'; ?>').value;
				if (protocol == "http://") {
					document.getElementById("<?php echo $prefix.'_ccheck_button_auto'; ?>").disabled = false;
				} else {
					document.getElementById("<?php echo $prefix.'_ccheck_button_auto'; ?>").disabled = true;
				}
				document.getElementById('<?php echo $prefix.'_ccheck_status_text'; ?>').innerHTML = "";

			}//end protocolChanged()


			function testUrl()
			{
				window.open(getFullUrl());

			}
		</script><?php

		$dd_values = array_flip($this->_edit_params['protocols']);
		foreach ($dd_values as $key => $value) {
			if ($key == 'mailto') {
				$dd_values[$key.':'] = $key.':';
			} else {
				$dd_values[$key.'://'] = $key.'://';
			}
			unset($dd_values[$key]);
		}

		combo_box($prefix.'_ccheck_protocol_dropdown', $dd_values, false, '', 0, 'onChange="protocolChanged()";');
		echo '&nbsp;';

		text_box($prefix.'_ccheck_input_box', $this->value, $this->_edit_params['width']);
		echo '&nbsp;';

		normal_button($prefix.'_ccheck_button_auto', 'Auto-Test', 'javascript: checkUrlExists();');
		normal_button($prefix.'_ccheck_button_manual', 'Open URL', 'javascript: testUrl();');


		?><div id="<?php echo $prefix; ?>_ccheck_status_text"></div><?php

		return true;

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		if (!isset($_REQUEST[$prefix.'_ccheck_input_box'])) return false;
		$value = (string) $_REQUEST[$prefix.'_ccheck_protocol_dropdown'].$_REQUEST[$prefix.'_ccheck_input_box'];
		if ($this->value != $value && $this->setValue($value)) {
			$this->processed = true;
		} else {
			$this->processed = false;
		}

	}//end process()


	/**
	* Ensure the attribute being set is a string
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		$new_value = (string) $value;
		if ($new_value != $value) return false;
		$value = $new_value;
		return true;

	}//end validateValue()


	/**
	* Retrieve a list of indexable values from this attribute
	*
	* The return value not only contains the indexable values, but also the number of
	* times they occur in this attributes value
	*
	* @return array(string => int)
	* @access public
	*/
	function getKeyData()
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		return get_word_counts(strtolower($this->value));

	}//end getKeyData()


}//end class

?>
