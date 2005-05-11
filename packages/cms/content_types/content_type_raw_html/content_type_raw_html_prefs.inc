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
* $Id: content_type_raw_html_prefs.inc,v 1.6 2005/05/11 06:07:30 lwright Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Content_Type_Raw_HTML_Prefs
*
* Purpose
*
*    Looks after the creation the Raw HTML preferences
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix
*/
class Content_Type_Raw_HTML_Prefs extends Prefs
{

	var $pref_vars = Array (
		'SQ_RAW_HTML_SIZE'   => Array(	'name'			=> 'Raw HTML Size',
										'description'	=> 'This preference allows you to change the size of the Raw HTML editor',
										'default'		=> Array('width' => '100', 'height' => '20'),
										'protected'		=> false),
	);


	/**
	* Constructor
	*
	*/
	function Content_Type_Raw_HTML_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these preferences or not
	* @param string						$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=null)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());

		if (is_null($pref) || $pref == 'SQ_RAW_HTML_SIZE') {
			if (is_null($pref)) $o->openField(translate('cms_ct_raw_html_pref_editor_size'));
				$html_size = $this->pref_vars['SQ_RAW_HTML_SIZE']['default'];
				?>
				<table class="sq-backend-table" style="width: 350px;">
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('width') ?></td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							text_box('prefs[SQ_RAW_HTML_SIZE][width]', $html_size['width']);
						} else {
							echo $html_size['width'];
						}
						?>
						</td>
					</tr>
					<tr>
						<td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('height') ?></td>
						<td class="sq-backend-table-cell" style="width: 100%;">
						<?php
						if ($have_lock && $is_admin) {
							text_box('prefs[SQ_RAW_HTML_SIZE][height]', $html_size['height']);
						} else {
							echo $html_size['height'];
						}
						?>
						</td>
					</tr>
				</table>
				<?php
			if (is_null($pref)) $o->closeField();
		}

		return true;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	* @param boolean					$have_lock	do we have the lock to edit these preferences or not
	* @param string						$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=null)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return false;

		// WYSIWYG STYLE
		if (is_null($pref) || $pref == 'SQ_RAW_HTML_SIZE') {
			if (isset($_POST['prefs']['SQ_RAW_HTML_SIZE'])) {
				$this->pref_vars['SQ_RAW_HTML_SIZE']['default'] = $_POST['prefs']['SQ_RAW_HTML_SIZE'];
			}
		}

		return true;

	}//end processBackend()


}//end class

?>