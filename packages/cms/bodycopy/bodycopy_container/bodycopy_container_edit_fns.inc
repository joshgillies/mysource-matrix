<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_container_edit_fns.inc,v 1.1 2003/11/03 01:22:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* Bodycopy_Container_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Bodycopy_Container_Edit_Fns extends Asset_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Bodycopy_Container_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['settings']);

	}//end constructor


	/**
	* Set an attribute value in the attributes var
	*
	* Internal checking of these attributes has to go here as they will not be checked by the system
	*
	* @param array	&$attributes	a reference to the attributes var of the BodyCopy_Container object
	* @param string	$name			the name of the attribute
	* @param string	$value			the value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttribute(&$attributes, $name, $value)
	{
		$name = strtolower($name);
		if (empty($attributes[$name])) $attributes[$name] = '';
		if ($attributes[$name] == $value) return false;
		$attributes[$name] = $value;
		return true;

	}//end setAttribute()

	
	/**
	* Prints the JavaScript needed by the backend functions for this table
	*
	* @param object Bodycopy_Container	&$asset	the container object being printed
	* @param string						$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	function paintBackendJsData(&$asset, $prefix)
	{
	
	}//end paintBackendJsData()


	/**
	* Paint JS data that containers need to use to do their processing
	*
	* @param object Bodycopy			&$bodycopy	the bodycopy being printed
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintGenericBackend(&$bodycopy, &$o, $prefix)
	{

	}//end paintGenericBackend()


	/**
	* Paints this container as HTML
	*
	* @param object Bodycopy_Container	&$asset		the container whose interface we are painting
	* @param booelan					$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $generating=false)
	{

	}//end paint()


	/**
	* Paint the interface for editing this container
	*
	* @param object Bodycopy_Container	$asset	the container whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintContainer(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('content')) {
			$this->paint($asset);
			return false;
		}
		return true;

	}//end paintContainer()


	/**
	* Process the interface for editing this container
	*
	* @param object BodyCopy_Container	&$asset		the container whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param string						$prefix		prefix for the form element
	* @param string						$bc_action	a global bodycopy action to perform
	* @param string						$bc_name	the name of the bodycopy that submitted the action
	* @param array						$bc_data	an array of data needed to perform the action
	* @param array						$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	function processContainer(&$asset, &$o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)
	{
		$updated = false;

		// save any changed attributes
		if (!empty($bc_saved[$prefix][$asset->id]['attributes'])) {
			$attributes_updated = false;
			$attributes = $asset->attr('attributes');
			foreach($bc_saved[$prefix][$asset->id]['attributes'] as $name => $value) {
				if ($name == 'identifier') {
					if ($asset->setAttrValue('name', $value)) $updated = true;
				} else {
					if ($this->setAttribute($attributes, $name, $value)) $attributes_updated = true;
				}
			}
			if ($attributes_updated) {
				$asset->setAttrValue('attributes',$attributes);
				$updated = true;
			}
		}

		return $updated;

	}//end processContainer()


	/**
	* Creates a PHP file to print on the frontend instead of doing processing there
	*
	* @param object Bodycopy_Container	&$asset	the container
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile(&$asset)
	{
		ob_start();
		$this->paint($asset, true);
		$output = ob_get_contents();
		ob_end_clean();

		$existing_links = Array();
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE);
		foreach ($notice_links as $link_data) $existing_links[] = $link_data['minorid'];

		$e = '/\\.\\/\\?a=([0-9]+)/';
		$matches = Array();
		preg_match_all($e, $output, $matches);
		$replace_assetids = $matches[1];

		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			// this container currently has a system version floating around
			// so we have to be careful not to delete notice links that are
			// still being used in this system version
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$system_assetids = unserialize(file_to_string($asset->data_path.'/.sq_system/.notice_links'));
			$replace_assetids = array_merge($replace_assetids, $system_assetids);
		}

		if (!empty($replace_assetids) || !empty($notice_links)) {

			// delete existing notice links that are no longer being used
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			foreach($notice_links as $link_data) {
				if (!in_array($link_data['minorid'], $replace_assetids)) {
					$asset->deleteLink($link_data['linkid']);
				}
			}
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			foreach($replace_assetids as $assetid) {
				if (!in_array($assetid, $existing_links)) {
					$ereg_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if ($ereg_asset->id) {
					
						// dont show errors here if the link already exists
						// as we rely on the fact that this ereg_asset cant be notice linked
						// to asset mutiple times to stop redunant links from
						// being created (eg same image on a page 10 times should only
						// produce one notice link)
						@$asset->createLink($ereg_asset, SQ_LINK_NOTICE);
					}
				}

				$e = "(/?\./\\?a={$assetid})([^0-9]+)";
				$with = '<?php echo $sq_asset_url_list['.$assetid.']; ?>';
				$output = ereg_replace($e, $with.'\\2', $output);
			}
		}

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

	}//end generateContentFile()


}//end class

?>