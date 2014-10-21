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
* $Id: tool_reindex.inc,v 1.13 2012/08/30 00:59:16 ewang Exp $
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
* @version $Revision: 1.13 $
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
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

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
	static function paintTool(&$o, $type_code)
	{
		$tool_info = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_code);
		$o->openSection($tool_info['name']);

		$o->openField('Root Node');
			asset_finder($type_code.'_reindexing_root_assetid', '');
			echo '&nbsp;&nbsp;&nbsp;<b>OR</b>';
		$o->closeField();

		$o->openField('System Wide');
			check_box($type_code.'_reindexing_root_folder');
			echo translate('Reindex all assets in the system');

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
	static function processTool(&$o, $type_code)
	{
		$index_all = 0;

		if (isset($_REQUEST[$type_code.'_reindexing_root_folder'])) {
			$index_all = $_REQUEST[$type_code.'_reindexing_root_folder'];
		}

		if ($index_all == 1) {
			// System-wide reindex
			$vars = Array('root_assetid' => $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_folder'));
		} else {
			$root_assetid = $_REQUEST[$type_code.'_reindexing_root_assetid']['assetid'];
			$vars = Array('root_assetid' => $root_assetid);
		}

		// Save the context we are in
		$vars['contextid'] = $GLOBALS['SQ_SYSTEM']->getContextId();

		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_reindex', $vars, '', SQ_PACKAGES_PATH.'/search/hipo_jobs');
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);
		return TRUE;

	}//end processTool()


}//end class


?>
