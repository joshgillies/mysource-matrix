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
* $Id: content_type_code_edit_fns.inc,v 1.12 2012/08/30 00:58:07 ewang Exp $
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
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Code_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

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
			text_area($prefix.'_html', $html, '', '', 0, 'style="width: '.$size['width'].'; height: '.$size['height'].'; font-family: \'Courier New\', Courier, monospace; display: block"');
			$line_number_prefs = $GLOBALS['SQ_SYSTEM']->getUserPrefs($asset->type(), 'SQ_CODE_EDITOR_LINE_NUMBERS');
			$current_line_settings = $asset->attr('line_numbers');
			foreach ($current_line_settings as $name => $value) {
				$line_number_prefs[$name] = $value;
			}
			?>
			<table class="sq-backend-table" style="width: 350px;">
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Show Line Numbers:'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						check_box($prefix.'_show_line_numbers', '1', $line_number_prefs['use_line_numbers']);
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Line Number Orientation:'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						radio_button($prefix.'_line_number_left', 'left', $line_number_prefs['line_number_left'] == TRUE);
						label(translate('Left'), $prefix.'_line_number_left');

						radio_button($prefix.'_line_number_left', 'right', $line_number_prefs['line_number_left'] == FALSE);
						label(translate('Right'), $prefix.'_line_number_left');

					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Line Number Style:'); ?>

					</td>
					<td class="sq-backend-table-cell" style="width: 100%;">
					<?php
						combo_box($prefix.'_line_style', Array('span'=>'Span','list'=>'List'), FALSE, $line_number_prefs['line_number_style']);
					?>
					</td>
				</tr>
				<tr>
					<td class="sq-backend-table-header" nowrap="nowrap" style="width: 100px;">
						<?php echo translate('Line Number Class:'); ?>

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
			$side = FALSE;
			if (isset($_POST[$prefix.'_line_number_left']) && $_POST[$prefix.'_line_number_left'] == 'left') {
				$side = TRUE;
			}
			if ($side != $line_number_prefs['line_number_left']) {
				$new_line_data['line_number_left'] = $side;
			}
			if (isset($_POST[$prefix.'_line_style']) && $_POST[$prefix.'_line_style'] != $line_number_prefs['line_number_style']) {
				$new_line_data['line_number_style'] = $_POST[$prefix.'_line_style'];
			} else {
				$new_line_data['line_number_style'] = 'span';
			}
			$new_line_data['line_number_class'] = '';
			if (isset($_POST[$prefix.'_line_class']) && $_POST[$prefix.'_line_class'] != $line_number_prefs['line_number_class']) {
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
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we genenerating the cache file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $editing=FALSE, $generating=FALSE)
	{
		// This may be changed to use the styles of the design, to show
		// what it will look like. If $editing == TRUE, then we do this
		$output =  $asset->attr('html');
		if ($generating) {
			// escape dangerous content
			require_once SQ_FUDGE_PATH.'/general/text.inc';
			escape_php($output);
		}
		echo $output;

	}//end paint()


}//end class

?>
