<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: content_type_nest_content_edit_fns.inc,v 1.8 2003/09/26 05:26:40 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../content_type/content_type_edit_fns.inc';

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
		if ($asset->writeAccess()) {
			$nested_assetid = $asset->attr('nested_assetid');
			$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', true);
			if ($nested_asset->id) {
				?> 
					<p style="font-size: 7pt">
					Currently nesting the contents of <i><?php echo $nested_asset->attr('name')?></i><br/>
					<?php asset_finder($prefix.'_nested_assetid', '', Array('page' => 'D')); ?><br/>
					<hr><b style="font-size: 7pt">The nested content is printed below</b><br/>
					</p>
				<?php
			} else {
				?> 
					<p style="font-size: 7pt">No page has been selected<br/>
					<?php asset_finder($prefix.'_nested_assetid', '', Array('page' => 'D')); ?>
					</p>
				<?php
			}
		}

		$this->paint($asset);
		return true;

	}//end paintBackend()


	/**
	* Just calls printBody on the nested asset and lets it take care of things
	*
	* @param object Content_Type_Nest_Content	&$asset	the nest content content type object
	*
	* @return boolean
	* @access public
	*/
	function paint(&$asset, $generating=false)
	{
		// get the asset - we cant pass the type in because it could be anything
		// we also mute errors because we check later on
		$nested_assetid = $asset->attr('nested_assetid');
		$nested_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($nested_assetid, '', true);
		if (!$nested_asset->id) return false;

		if ($generating) {
			$this->paintGenerated($asset, $nested_asset);
			return true;
		}

		// keep track of what we have printed
		//if(!isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'])) $GLOBALS['SQ_NEST_CONTENT_PAINTED'] = Array();

		// if we have already printed this asset, dont print again or we will end up in an endless loop
		if (isset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id])) {
			unset($GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id]);
			echo '<font color="red"><b>[ ** RECURSION WARNING ** ]<br/>You are nesting the content of this page inside itself</b></font>';
			return true;
		}

		// record that we have printed this asset
		// OKAY! so we havn't actually printed it yet, but we need to record that
		// we have or we will just fall endlessly into printBody functions and
		// never get back here again
		$GLOBALS['SQ_NEST_CONTENT_PAINTED'][$nested_asset->id] = 1;

		// and now actually print it
		$nested_asset->printBody();

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
		echo '<?php'."\n";
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
		echo '?>'."\n";

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
		$current_assetid = $asset->attr('nested_assetid');
		$assetid = (isset($_POST[$prefix.'_nested_assetid'])) ? $_POST[$prefix.'_nested_assetid'] : '';
		if (!empty($assetid)) {
			if ($assetid != $current_assetid) {
				if (!$asset->setAttrValue('nested_assetid', $assetid)) return false;
				
				// get the html contents of the nested asset so
				// id we change the type of the cell, other content
				// types can use the html
				ob_start();
				$this->paint($asset);
				$html = ob_get_contents();
				$asset->setAttrValue('html', $html);
				ob_end_clean();
				return true;
				
			}
		}
		return false;

	}//end processBackend()

}//end class

?>