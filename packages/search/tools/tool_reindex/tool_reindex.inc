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
* $Id: tool_reindex.inc,v 1.1 2005/03/16 05:21:08 ndvries Exp $
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
* @version $Revision: 1.1 $
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
	* @param  &$o        reference to the backend outputter of the object painting the tool
	* @param  $type_code the type code of the tool being painted
	*
	* @return String
	* @access public
	*/
	function paintTool(&$o, $type_code)
	{
		ob_start();

		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField('Root Node');
			asset_finder($type_code.'_reindexing_root_assetid', '');
			echo '&nbsp;&nbsp;&nbsp;<b>OR</b>';
		$o->closeField();

		$o->openField('System Wide');
			check_box($type_code.'_reindexing_root_folder');
			echo 'Reindex all assets in the system';
		$o->closeField();

		$o->openField('', 'commit');
			echo '&nbsp;';
			hidden_field('committed_tool_type_code', $type_code);
			submit_button('tool_commit', 'Commit');
		$o->closeField();

		$o->closeSection();
		return ob_get_clean();

	}//end paintTool()


	/**
	* Processes the user input and performs the function of the tool.
	*
	* @param  &$o        reference to the backend outputter of the object painting the tool
	* @param  $type_code the type code of the tool being painted
	*
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

		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_tool_reindex', $vars, '', SQ_PACKAGES_PATH.'/search/tools/tool_reindex');
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

	}//end processTool()


}//end class


?>
