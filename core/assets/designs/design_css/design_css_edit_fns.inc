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
* $Id: design_css_edit_fns.inc,v 1.2 2004/08/21 08:18:53 amiller Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design_edit_fns.inc';

/**
* Design_Css_Edit_Fns
*
* Purpose
*
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Edit_Fns extends Design_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Design_Css_Edit_Fns()
	{
		$this->Design_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor


	/**
	* Parses the parse file and processes the contents
	*
	* @param object Asset	$asset	the asset to which we belong
	*
	* @return boolean
	* @access public
	*/
	function parseAndProcessFile(&$asset)
	{
		$parse_file = $asset->data_path.'/parse.txt';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$str = file_to_string($parse_file);
		$contents = $this->_parseString($asset, $str);
		return (!is_null($contents) && $this->_processContents($asset, $contents) && $asset->saveAttributes());

	}//end parseAndProcessFile()
	
	
	/**
	* Parses the string into an array recursively cut up into html and tag pieces
	*
	* @param object Asset	$asset	the asset to which we belong
	* @param string	$file_contents	the contents to parse
	*
	* @return Array
	* @access private
	*/
	function _parseString(&$asset, $file_contents)
	{
		// Let's place an print operation at the top of the file
		// to allow us to print the default header stuff like
		// copyright and content-type information
		$file_contents= "<".$this->tag_name."_PRINT id_name='__global__' var='css_header' />\n"
						.$file_contents;
						
		return parent::_parseString($asset, $file_contents);

	}//end _parseString()
	
	
	/**
	* Creates a new file asset, populates it, and links it to $asset
	* @param object Asset	$asset	the asset to link the file to
	* @param array			$info	info about the uploaded file
	*
	* @return boolean
	* @access public
	*/
	function _processUploadedFile(&$asset, $info=Array())
	{
		// if we already have a file with the same name TYPE_3 linked to us,
		// delete this link so we link up this new file
		$existing_ids = Array();
		$existing = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, 'file', false);
		foreach ($existing as $link) $existing_ids[$link['minorid']] = $link['linkid'];
		$existing_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(array_keys($existing_ids));
		foreach ($existing_info as $existing_info) {
			// if the name is the same, delete the link
			if ($existing_info['name'] == $info['name']) {
				$linkid = $existing_ids[$existing_info['assetid']];
				$asset->deleteExistingLink($linkid);
				$asset->updateLookups();
			}
		}

		return parent::_processUploadedFile($asset, $info);

	}//end _processUploadedFile()
	
	
	/**
	* Paint the interface for the virtual path to the current css file
	*
	* @param object Asset				$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintVirtualPath(&$asset, &$o, $prefix)
	{
		echo 'mysource_css/'.$asset->attr('id_name');
		return false;
	}//end paintVirtualPath()
	
	
	/**
	* Creates the (hopefully ;) single accumulated design file for this design
	*
	* @param object Asset	$asset	the asset to which we belong
	*
	* @return boolean
	* @access private
	*/
	function createDesignFile(&$asset)
	{
		$design_area_links = $asset->getDesignAreaLink();

		// Firstly we need to set all the var_reference variables so that when the design areas are
		// printed they can work correctly
		//
		// This is a two stage process, one to set any normal vars and two to set any vars that want DA
		// output

		$var_ref_values = Array();
		$da_refs = Array();
		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;

			$da_var_refs = $da->getVarReferences();
			for(reset($da_var_refs); null !== ($id_name = key($da_var_refs)); next($da_var_refs)) {
				$var_da = &$asset->getDesignArea($id_name);
				if (is_null($var_da)) continue;
				if (!isset($var_ref_values[$id_name])) $var_ref_values[$id_name] = Array();
				foreach($da_var_refs[$id_name] as $var) {
					// does this var want the whole design area ?
					if ($var === '') {
						$da_refs[] = $id_name;
					// else just a normal var (make sure we don't have it already)
					} elseif (!isset($var_ref_values[$id_name][$var])) {
						$var_ref_values[$id_name][$var] = $var_da->attr($var);

					}// end if

				}// end foreach

			}// end foreach

		}// end foreach

		// OK, now that we have all the var reference values we can just set them for all the DA's
		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$da->setVarReferenceValues($var_ref_values);
		}// end foreach

		// Now that all the all the DA's have their vars set, we can do the DA output ones
		$da_ref_values = Array();
		foreach($da_refs as $id_name) {
			$da = &$asset->getDesignArea($id_name);
			if (is_null($da)) continue;
			ob_start();
			$da->printArea($asset);
			// NOTE: format like this because we are calling setVarReferenceValues()
			$da_ref_values[$id_name] = Array('' => ob_get_contents());
			ob_end_clean();
		}// end foreach

		// OK, now that we have all the da reference values we can just set them for all the DA's
		foreach($design_area_links as $link) {
			$da = &$asset->getDesignArea($link['value']);
			if (is_null($da)) continue;
			$da->setVarReferenceValues($da_ref_values);
		}// end foreach

		// we are checking to see if there is a global $ASSET_LINEAGE here
		// because we need other assets to be able to modify the lineage, for
		// instance 404 pages.
		$pre_str = '<?php
		header("Content-type: text/css");
		global $ASSET_LINEAGE;
		$am = &$GLOBALS[\'SQ_SYSTEM\']->am;
		if (!isset($ASSET_LINEAGE) || empty($ASSET_LINEAGE)) $ASSET_LINEAGE = @$am->getLineageFromURL(null, null);
		define(\'SQ_LIMBO_EDITING\', (SQ_IN_LIMBO && (!isset($_REQUEST[\'ignore_frames\']) || !$_REQUEST[\'ignore_frames\'])));
		?>';

		$str = '';
		ob_start();
		if ($this->_createDesignFileOutput($asset->attr('contents'), $asset, $asset)) {
			// get the contents, removing any close then open tags that happen right next to each other
			// (more for code neatness than anything :)
			$str = str_replace('?'.'><'.'?php', '', ob_get_contents());
		}
		ob_end_clean();
		if (!$str) return false;
		$str = $pre_str."\n".$str;

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		create_directory($asset->data_path);
		$file = $asset->data_path.'/design_file.php';
		$new_file = !file_exists($file);
		if (!string_to_file($str, $file)) return false;


		//// FILE VERSIONING ////

		$fv = &$GLOBALS['SQ_SYSTEM']->getFileVersioning();

		if ($new_file) {
			// attempt to add the file to the repository
			if (!$fv->add($asset->data_path_suffix, $file, '', false)) {
				trigger_error('Failed adding design file version', E_USER_WARNING);
				return false;
			}
		} else {
			// updating existing file
			$file_status = $fv->upToDate($file);
			if (FUDGE_FV_MODIFIED & $file_status) {
				if (!$fv->commit($file, '', false)) {
					trigger_error('Failed committing design file version', E_USER_WARNING);
					return false;
				}
			}
		}

		// make sure we have the latest version of our file
		if (!$fv->checkOut($asset->data_path_suffix.'/design_file.php', $asset->data_path)) {
			trigger_error('Failed checking out latest version of design file', E_USER_WARNING);
			return false;
		}

		return true;

	}//end createDesignFile()

}//end class

?>
