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
* $Id: content_type_nest_content_edit_fns.inc,v 1.22 2004/07/02 10:58:42 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/content_type/content_type_edit_fns.inc';

/**
* Content_Type_Nest_Content_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Content_Type_Nest_Content_Edit_Fns extends Content_Type_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Content_Type_Nest_Content_Edit_Fns()
	{
		$this->Content_Type_Edit_Fns();

	}//end constructor


	/**
	* Prints the backend interface for this WYSIWYG editor
	*
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	* @param string								$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$asset, $prefix)
	{
		if ($asset->writeAccess('links') && $asset->writeAccess('attributes')) {
			$nested_asset = &$this->_getNestedAsset($asset);

			if (!is_null($nested_asset)) {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-cell-alt"><b>Currently nesting the contents of "<i><?php echo $nested_asset->attr('name')?></i>"</b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', $nested_asset->id, Array('page' => 'D')); ?></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><b>The nested content is printed below</b></td>
					</tr>
					<tr>
						<td style="border-top: 1px solid black"><br/><?php $this->paint($asset, $prefix);?><br/></td>
					</tr>
				</table>
				<?php
			} else {
				?>
				<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-cell-alt"><b>No page has been selected</b></td>
					</tr>
					<tr>
						<td class="sq-backend-table-cell"><?php asset_finder($prefix.'_nested_assetid', '', Array('page' => 'D')); ?></td>
					</tr>
				</table>
				<?php
			}
		}

		return true;

	}//end paintBackend()


	/**
	* Just calls printBody on the nested asset and lets it take care of things
	*
	* @param object Content_Type_Nest_Content	&$asset		the nest content content type object
	* @param string								$prefix		prefix for form elements
	* @param boolean							$generating	are we genenerating the cache file
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $prefix, $generating=false)
	{
		// get the asset - we cant pass the type in because it could be anything
		// we also mute errors because we check later on
		$nested_asset = &$this->_getNestedAsset($asset);
		if (is_null($nested_asset)) return false;

		if ($generating) {
			$this->paintGenerated($asset, $nested_asset);
			return true;
		}

		// keep track of what we have printed

		// if we have already printed this asset, dont print again or we will end up in an endless loop
		if (isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id])) {
			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id]);
			echo '<span class="sq-backend-warning"><b>[ ** RECURSION WARNING ** ]<br/>You are nesting the content of this page inside itself</b></span>';
			return true;
		}

		// record that we have printed this asset
		// OKAY! so we havn't actually printed it yet, but we need to record that
		// we have or we will just fall endlessly into printBody functions and
		// never get back here again
		$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id] = 1;

		// and now actually print it

		ob_start();
		$nested_asset->printBody();
		$asset_contents = ob_get_contents();
		ob_end_clean();

		$form_tags = Array('form', 'input' , 'select', 'option', 'textarea');
		foreach($form_tags as $tag) {
			if (stristr($asset_contents, '<'.$tag) !== false) {
			?>
				<span class="sq-backend-warning">
					<b>NOTE:</b> this nested asset contains form elements, therefore you cannot view its 
					content in the backend.<br/>You can only view the nested content by previewing on the frontend.
				</span>
			<?php
				unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id]);
				return true;
			}#end if
		}#end for

		echo $asset_contents;

		unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id]);

		return true;

	}//end paint()


	/**
	* Paints the content type as it should be cached
	*
	* @param object Bodycopy_Table	$asset	the table whose interface we are painting
	*
	* @return void
	* @access public
	*/
	function paintGenerated(&$asset, &$nested_asset)
	{
		echo '<'.'?php'."\n";
		?>
		if(!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'])) $GLOBALS['SQ_NEST_CONTENT_PAINTED'] = Array();

		if (!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][<?php echo $nested_asset->id?>])) {
			$GLOBALS['SQ_NEST_CONTENT_PAINTED'][<?php echo $nested_asset->id?>] = 1;
			$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset(<?php echo $nested_asset->id?>, '', true);
			if ($nested_asset->id) $nested_asset->printBody();
		} else {
			echo '<font color="red"><b>[ ** RECURSION WARNING ** ]<br/>You are nesting the content of this page inside itself</b></font>';
		}

		unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][<?php echo $nested_asset->id?>]);
		<?php
		echo '?'.'>'."\n";

	}//end paintGenerated()


	/**
	* Processes the backend interface for this WYSIWYG editor
	*
	* @param array(string)						$link	information used to create the initial link
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	* @param string								$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processBackend($link, &$asset, $prefix)
	{
		$nested_asset = &$this->_getNestedAsset($asset);
		$current_assetid = (!is_null($nested_asset)) ? $nested_asset->id : 0;
		$assetid = (isset($_POST[$prefix.'_nested_assetid']['assetid'])) ? (int)$_POST[$prefix.'_nested_assetid']['assetid'] : 0;

		if ($assetid) {

			if ($assetid != $current_assetid) {

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

				if ($current_assetid) {
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', true, 'nested_asset');
					if (!$asset->deleteLink($link['linkid'])) {
						$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
						return false;
					}
				}

				// create a 'hidden' link for delete notification
				$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (!$asset->createLink($nested_asset, SQ_LINK_NOTICE, 'nested_asset')) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}

				// get the html contents of the nested asset so
				// we change the type of the cell later - other content
				// types can use the html
				ob_start();
					$this->paint($asset, $prefix);
					$html = ob_get_contents();
					$asset->setAttrValue('html', $html);
				ob_end_clean();

				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
				return true;

			}

		} else if ($current_assetid) {
			$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', true, 'nested_asset');
			return $asset->deleteLink($link['linkid']);
		}
		return false;

	}//end processBackend()


	/**
	* Get a reference to the asset the content type is nesting
	*
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	*
	* @return object Asset | NULL
	* @access private
	*/
	function &_getNestedAsset(&$asset)
	{
		$nested_link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', true, 'nested_asset');
		if (!empty($nested_link)) {
			return $GLOBALS['SQ_SYSTEM']->am->getAsset($nested_link['minorid'], $nested_link['minor_type_code'], true);
		} else {
			return null;
		}

	}//end _getNestedAsset()


}//end class

?>
