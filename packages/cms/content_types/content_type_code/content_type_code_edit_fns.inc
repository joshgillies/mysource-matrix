<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: content_type_code_edit_fns.inc,v 1.3.2.1 2006/05/30 05:18:16 rhoward Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Code_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.3.2.1 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Code_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Code_Edit_Fns()
	{
		$this->Content_Type_Edit_Fns();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object	&$asset	The Code Content Type Object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$size = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CODE_EDITOR_SIZE');
			$html = $asset->attr('edit_content');
			$html = str_replace('&lt;?php', '<?php', $html);
			$html = str_replace('&lt;script', '<script', $html);
			text_area($prefix.'_html', $html, '', '', 0, 'style="width: '.$size['width'].'; height: '.$size['height'].'; font-family: monospace; display: block"');
			$line_number_prefs = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CODE_EDITOR_LINE_NUMBERS');
			$current_line_settings = $asset->attr('line_numbers');
			foreach ($current_line_settings as $name => $value) {
				$line_number_prefs[$name] = $value;
			}
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('show_line_numbers'); ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						check_box($prefix.'_show_line_numbers', '1', $line_number_prefs['use_line_numbers']);
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('line_number_orientation'); ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						radio_button($prefix.'_line_number_left', 'left', $line_number_prefs['line_number_left'] == TRUE);
						label(translate('left'), $prefix.'_line_number_left');
						radio_button($prefix.'_line_number_left', 'right', $line_number_prefs['line_number_left'] == FALSE);
						label(translate('right'), $prefix.'_line_number_left');
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('line_number_style'); ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						combo_box($prefix.'_line_style', Array('span'=>'Span','list'=>'List'), FALSE, $line_number_prefs['line_number_style']);
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('line_number_class'); ?>
					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						text_box($prefix.'_line_class', $line_number_prefs['line_number_class']);
					?>
					</td>
				</tr>
			</table>
			<?php
		}//end if write access
		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface for this Raw HTML editor
	*
	* @param array	$link	information used to create the initial link
	* @param object	&$asset	the wysiwyg content type object
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		if ($asset->writeAccess('attributes')) {
			$line_number_prefs = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CODE_EDITOR_LINE_NUMBERS');
			$new_line_data = Array();
			// We only want to store new line number details, if they differ from the global prefs
			if (isset($_POST[$prefix.'_show_line_numbers']) != $line_number_prefs['use_line_numbers']) {
					$new_line_data['use_line_numbers'] = isset($_POST[$prefix.'_show_line_numbers']);
			}
			$side = ($_POST[$prefix.'_line_number_left'] == 'left') ? TRUE : FALSE;
			if ($side != $line_number_prefs['line_number_left']) {
				$new_line_data['line_number_left'] = $side;
			}
			if ($_POST[$prefix.'_line_style'] != $line_number_prefs['line_number_style']) {
				$new_line_data['line_number_style'] = $_POST[$prefix.'_line_style'];
			}
			if ($_POST[$prefix.'_line_class'] != $line_number_prefs['line_number_class']) {
				$new_line_data['line_number_class'] = $_POST[$prefix.'_line_class'];
			}
			$asset->setAttrValue('line_numbers', $new_line_data);

			if (isset($_POST[$prefix.'_html'])) {
				$html = $_POST[$prefix.'_html'];

				$asset->setAttrValue('edit_content', str_replace(Array('<script','<?php'), Array('&lt;script', '&lt;?php'), $html));

				$html = $asset->syntaxHighlight($html);

				$current_html = $asset->attr('html');
				if ($html != $current_html) {
					if (!$asset->setAttrValue('html', $html)) return FALSE;
				}
			}

		}//end if write access
		return $asset->writeAccess('attributes');

	}//end processBackend()


	/**
	* Paints raw HTML to be printed on the frontend
	*
	* @param object		&$asset		the content type we are painting
	* @param string		$prefix		prefix for form elements
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $prefix, $editing=FALSE, $generating=FALSE)
	{
		// This may be changed to use the styles of the design, to show
		// what it will look like. If $editing == TRUE, then we do this
		$output =  $asset->attr('html');
		if ($generating) {
			// escape dangerous content, eg. <?php, <? - leaves <?xml intact and working
			$output = str_replace('<?', "<?php echo '<'.'?'; ?>", $output);
		}
		echo $output;

	}//end paint()


}//end class

?>
