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
* $Id: design_area_body.inc,v 1.25 2005/12/08 00:58:56 rhoward Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Body
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.25 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Body extends Design_Area
{


	/**
	* Print code that actually results in HTML output to the browser for this design area
	*
	* This function is called at the AREA tag if print != no, and at the point of any PRINT
	* tags that reference this area.
	*
	* @return void
	* @access protected
	*/
	function _printPaintingCode()
	{
		$format = $this->attr('format');

		echo '
		<','?php
		$body = \'\';
		if (SQ_IN_LIMBO) {
			ob_start();
				$ASSET->printLimbo();
				$body = ob_get_contents();
			ob_end_clean();
		} else {
			if ($ASSET->_is_cacheable) {
				$cm = &$GLOBALS[\'SQ_SYSTEM\']->am->getSystemAsset(\'cache_manager\');
				$body = $cm->loadFromCache($ASSET->id, $ASSET->type());
			}
			if (empty($body)) {

				ob_start();

				$url = strip_url(current_url(false, true));

				// Have they set a paint layout name that they want to view ??
				if (!empty($_GET[\'SQ_PAINT_LAYOUT_NAME\'])) {
					$layout_name = \'paint_layout::user::\'.$_GET[\'SQ_PAINT_LAYOUT_NAME\'];
				// is it in the session ?
				} elseif (!empty($_SESSION[\'SQ_PAINT_LAYOUT_NAME\'])) {
					$layout_name = \'paint_layout::user::\'.$_SESSION[\'SQ_PAINT_LAYOUT_NAME\'];

				// Maybe they have a design name that we can use to match against
				} elseif (!empty($_GET[\'SQ_DESIGN_NAME\'])) {
					$layout_name = \'paint_layout::user::\'.$_GET[\'SQ_DESIGN_NAME\'];
				} elseif (!empty($_SESSION[\'SQ_DESIGN_NAME\'])) {
					$layout_name = \'paint_layout::user::\'.$_SESSION[\'SQ_DESIGN_NAME\'];

				} else {
					$layout_name = \'paint_layout::system::frontend\';

				}

				$result = $GLOBALS[\'SQ_SYSTEM\']->am->getValueFromURL($url, $layout_name);

				// if no layout was found, try and get the default frontend
				// layout if we were not already trying to get it
				if (empty($result) && $layout_name != \'paint_layout::system::frontend\') {
					$result = $GLOBALS[\'SQ_SYSTEM\']->am->getValueFromURL($url, \'paint_layout::system::frontend\');
				}

				$global_def_layout_id = $GLOBALS[\'SQ_SYSTEM\']->getGlobalDefine(\'SQ_PAINT_LAYOUT_ID\', 0);
				if (!$result && $global_def_layout_id) {
					$result = $global_def_layout_id;
				}

				if ($result) {
					// we have found the design to use
					$layout = &$GLOBALS[\'SQ_SYSTEM\']->am->getAsset($result);
					$layout->paint($ASSET);
				} else {
					// we can\'t find a layout, oh well let\'s just print out our body
					$ASSET->printBody();
				}

				$body = ob_get_clean();
			}
		';

		switch ($format) {
			case 'text':
			case 'low_bandwidth':
				// text only design - strip images, embeds and objects
				echo '
					$body = preg_replace(\'|\<img([^\>]*)\>|Ui\',\'\',$body);
					$body = preg_replace(\'|\\<embed(.*)\>(.*)</embed>|i\',\'\',$body);
					$body = preg_replace(\'|\\<object(.*)\>(.*)</object>|i\',\'\',$body);
					';
				if ($format == 'low_bandwidth') {
					// low bandwidth design - strip images, embeds, objects, and additional whitespace
					echo '
						$body = preg_replace(\'|\s+|\', \' \', $body);
						$body = str_replace(\'<br />\', \'<br/>\', $body);
						$body = str_replace(\'<br/>\', "\n", $body);
						$body = preg_replace(\'|\n\s+|\', "\n", $body);
						$body = preg_replace(\'|[\n]+|\', "\n", $body);
						$body = str_replace("\n", \'<br/>\', $body);
						';
				}
			break;
		}

		echo '
		}
		// global keyword replacements
		if (!empty($body)) {
			$body = replace_global_keywords($body);
		}

		echo $body;
		';
		echo '
		?','>';
		return true;

	}//end _printPaintingCode()


	/**
	* Returns true if this content generated by this design area can be cached
	*
	* @return boolean
	* @access public
	*/
	function canBeCached()
	{
		return false;

	}//end canBeCached()


}//end class
?>