<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: date_list.inc,v 1.5 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_ATTRIBUTES_PATH.'/datetime/datetime.inc';

/**
* Date List Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <date_list />
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_Date_List extends Asset_Attribute_Datetime
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_Date_List($attributeid=0, $value=null)
	{
		$this->Asset_Attribute_Datetime($attributeid, $value);
		if (is_null($value)) {
			$this->value = '---------- --:--:--';
		}

	}//end constructor


	/**
	* Print the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form elements
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		$prefix = str_replace(':','_',$prefix); // handle shadow assets
		if (empty($this->value)) {
			$value = Array('---------- --:--:--');
		} else {
			$value = explode("\r\n", $this->value);
		}
		$dta = new Asset_Attribute_Datetime();
		$dta->_params = $this->_params;
		$dta->_edit_params = $this->_edit_params;
		$dta->_edit_params['null'] = Array('y', 'm', 'd', 'h', 'i', 's');
		$dta->value = '---------- --:--:--';
		ob_start();
			$dta->paint($prefix, $read_only);
			$dta_template = ob_get_contents();
		ob_end_clean();
		$dta_template = preg_replace('/value\[([^"]+)\]/', '_value[\\1][]', $dta_template);
		if (!$read_only) {
			?>
			<div id="date-list-<?php echo $prefix; ?>">
				<?php
				foreach ($value as $v) {
					echo '<span>';
					$dta->value = $v;
					ob_start();
						$dta->paint($prefix, $read_only);
						$dta_output = ob_get_contents();
					ob_end_clean();
					echo preg_replace('/value\[([^"]+)\]/', '_value[\\1][]', $dta_output);
					echo '</span>';
					?><button type="button" tabindex="99999" class="delete-button">&nbsp;</button><br />
					<?php
				}
				for ($i =0; $i < 2; $i++) {
					echo '<span>'.$dta_template.'</span>';
					?><button type="button" tabindex="99999" class="delete-button">&nbsp;</button><br />
					<?php
				}
				?>
			</div>
			<script type="text/javascript" src="<?php echo sq_web_path('lib'); ?>/js/edit.js"></script>
			<script type="text/javascript">
				// attach the event handlers
				attachDateListEventHandlers(document.getElementById('date-list-<?php echo $prefix; ?>'));
			</script>
			<?php
		} else {
			echo '<ul>';
			foreach ($value as $v) {
				echo '<li>'.date($this->_edit_params['print_format'], iso8601_ts($v)).'</li>';
			}
			echo '</ul>';
		}

	}//end paint()


	/**
	* Process the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form elements
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$prefix = str_replace(':','_',$prefix);	// handle shadow assets
		if (!isset($_POST[$prefix.'show'])) return;
		$post = $_REQUEST[$prefix.'_value'];
		$keys = array_keys(reset($post));
		foreach ($keys as $key) {
			$elt = sprintf('%04d-%02d-%02d %02d:%02d:%02d', $post['y'][$key], $post['m'][$key], $post['d'][$key], $post['h'][$key], $post['i'][$key], $post['s'][$key]);
			if ($elt != '0000-00-00 00:00:00') {
				$value[] = $elt;
			}
		}
		$this->processed = $this->setValue(implode("\r\n", $value));

	}//end process()


	/**
	* Ensure the attribute being set is valid
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		foreach (explode("\r\n", $value) as $v) {
			if (!parent::validateValue($v)) return false;
		}
		return true;

	}//end validateValue()


}//end class

?>
