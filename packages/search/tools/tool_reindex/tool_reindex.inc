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
* $Id: tool_reindex.inc,v 1.7 2006/05/05 06:48:43 tbarrett Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Reindex
*
* Purpose
*
*    Tool for reindexing assets so that the Search Manager can use the index for
*    searching.
*
* @author Nathan de Vries <ndvries@squiz.net>
* @author David Heppell <dheppell@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage search
*/

class Tool_Reindex extends Tool
{


	/**
	* Constructor
	*
	* @param int  $assetid  the asset id to be loaded
	*
	*/
	function Tool_Reindex($assetid=0)
	{
		$this->Tool($assetid);

	}//end constructor


	/**
	* Paints the tool interface.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return void
	* @access public
	*/
	function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField('Root Node');
			asset_finder($type_code.'_reindexing_root_assetid', '');
			echo '&nbsp;&nbsp;&nbsp;<b>OR</b>';
		$o->closeField();

		$o->openField('System Wide');
			check_box($type_code.'_reindexing_root_folder');
			echo translate('sch_reindex_all_assets');
		$o->closeField();

		$o->closeSection();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param object	&$o			reference to the backend outputter of the object painting the tool
	* @param string	$type_code	the type code of the tool being painted
	*
	* @return boolean
	* @access public
	*/
	function processTool(&$o, $type_code)
	{
		$index_all = 0;

		if (isset($_REQUEST[$type_code.'_reindexing_root_folder'])) {
			$index_all = $_REQUEST[$type_code.'_reindexing_root_folder'];
		}

		if ($index_all == 1) {
			$vars = Array('root_assetid' => '1');
		} else {
			$root_assetid = $_REQUEST[$type_code.'_reindexing_root_assetid']['assetid'];
			$vars = Array('root_assetid' => $root_assetid);
		}

		$hh =& $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_reindex', $vars, '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


}//end class


?>
