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
* $Id: tool_mass_clone.inc,v 1.7 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Mass_Clone
*
* Purpose
* Provides a tool for making multiple clones of an asset
*
* @author Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage core
*/

class Tool_Mass_Clone extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Mass_Clone($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paint the interface for this tool
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this tool asset
	*
	* @return void
	* @access public
	* @static
	*/
	public static function paintTool(&$o, $type_code)
	{
		$o->openSection(translate('mass_clone'));
			$o->openField(translate('mass_clone_asset_to_clone'));
				asset_finder('asset_to_clone', 0, call_user_func(Array($type_code, 'getClonableTypes')));
				$err_msg = str_replace("'", "\'", translate('mass_clone_must_choose_asset'));
				$o->addOnSubmit("if (document.getElementById('asset_to_clone[assetid]').value == 0) {
					alert('$err_msg');
					return false;
				}");
			$o->closeField();
			$o->openField(translate('mass_clone_num_clones'));
				int_text_box('number_of_clones', 1, FALSE, 3, 1, 128);
			$o->closeField();
			$o->openField('Parent for new clones');
				asset_finder('clone_parent');
				$err_msg = str_replace("'", "\'", translate('mass_clone_must_choose_parent'));
				$o->addOnSubmit("if (document.getElementById('clone_parent[assetid]').value == 0) {
					alert('$err_msg');
					return false;
				}");
			$o->closeField();
			$o->openField(translate('mass_clone_type_of_link'));
				$link_types = get_link_type_names();
				unset($link_types[SQ_LINK_NOTICE]);
				combo_box('link_type', $link_types, FALSE, '');
			$o->closeField();
		$o->closeSection();

	}//end paintTool()


	/**
	* Process this tool.  Return false if any errors.
	*
	* @param object	&$o			reference to the outputter object
	* @param string	$type_code	type code of this asset
	*
	* @return boolean
	* @access public
	* @static
	*/
	public static function processTool(&$o, $type_code)
	{
		if (!isset($_REQUEST['asset_to_clone']) || empty($_REQUEST['asset_to_clone']['assetid'])) {
			trigger_localised_error('CORE0253', E_USER_WARNING);
			return FALSE;
		}
		if (!isset($_REQUEST['clone_parent']) || empty($_REQUEST['clone_parent']['assetid'])) {
			trigger_localised_error('CORE0254', E_USER_WARNING);
			return FALSE;
		}
		if (empty($_REQUEST['number_of_clones'])) {
			trigger_localised_error('CORE0255', E_USER_WARNING);
			return FALSE;
		}
		if (empty($_REQUEST['link_type'])) {
			trigger_localised_error('CORE0256', E_USER_WARNING);
			return FALSE;
		}
		$vars = call_user_func(Array($type_code, 'getHipoRunningVars'), $type_code);
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_clone_assets', $vars);
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


	/**
	* Get a list of typecodes that can be cloned by this tool, in the format passed to html_form::asset_finder
	*
	* @return array
	* @access protected
	* @static
	*/
	public static function getClonableTypes()
	{
		return Array();

	}//end getClonableTypes()


	/**
	* Get the running vars to pass to the HIPO
	*
	* @param string	$type_code	The type code of this tool asset
	*
	* @return array
	* @access protected
	* @static
	*/
	public static function getHipoRunningVars($type_code)
	{
		$original_id = $_REQUEST['asset_to_clone']['assetid'];
		$num_clones = $_REQUEST['number_of_clones'];
		return Array(
				'assets'			=> Array($original_id => Array('number_of_clones' => $num_clones)),
				'to_parent_assetid'	=> $_REQUEST['clone_parent']['assetid'],
				'to_parent_pos'		=> -1,
				'link_type'			=> $_REQUEST['link_type'],
				'value'				=> '',
				'finalise_function'	=> Array($type_code, 'finaliseClone'),
			   );

	}//end getHipoRunningVars()


	/**
	* Perform any additional processing required on each cloned asset
	*
	* This is passed to the clone_assets HIPO as a callback, and is called by the HIPO
	* (statically) after each clone is created.
	*
	* @param object	&$clone			The cloned asset
	* @param array	&$running_vars	The running vars of the clone-assets hipo (can be used for storage)
	*
	* @return void
	* @access public
	* @static
	*/
	public static function finaliseClone(&$clone, &$running_vars)
	{
		// child classes can override

	}//end finaliseClone()


}//end class
?>
