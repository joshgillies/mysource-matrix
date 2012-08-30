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
* $Id: tool_change_thesaurus_term.inc,v 1.3 2012/08/30 01:09:12 ewang Exp $
*
*/

require_once SQ_SYSTEM_ROOT.'/core/assets/system/tool/tool.inc';

/**
* Tool_Change_Thesaurus_Term
*
* Purpose
* Provides a tool for updating metadata for thesaurus terms
*
* @author Benjamin Pearson <bpearson@squiz.net>
* @version $Revision: 1.3 $
* @package MySource_Matrix_Packages
* @subpackage core
*/

class Tool_Change_Thesaurus_Term extends Tool
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Tool_Change_Thesaurus_Term($assetid=0)
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
		$o->openSection(translate('tool_rename_thesaurus_term'));
			$o->openField(translate('tool_thesaurus_term'));
				asset_finder('thesaurus_term', 0, Array('thesaurus_term'=>'I'));
				$err_msg = str_replace("'", "\'", translate('tool_thesaurus_term_empty'));
				$o->addOnSubmit("if (document.getElementById('thesaurus_term[assetid]').value == 0) {
					alert('$err_msg');
					return false;
				}");
			$o->closeField();
			$o->openField(translate('tool_thesaurus_term_old'));
				text_box('thesaurus_term_old');
				$err_msg = str_replace("'", "\'", translate('tool_thesaurus_term_old_empty'));
				$o->addOnSubmit("if (document.getElementById('thesaurus_term_old').value == '') {
					alert('$err_msg');
					return false;
				}");
			$o->closeField();
			// No need for asking for the thesaurus, they pick the term, we know the thesaurus it comes from
			$o->openField(translate('tool_thesaurus_term_root_node'));
				asset_finder('root_node', 0);
				echo '<br />';
				check_box('cascade', '1', FALSE);
				label(translate('tool_thesaurus_term_cascade'), 'cascade');
			$o->closeField();
			$o->sectionNote(translate('tool_rename_thesaurus_term_note'));
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
		$current_term 	= array_get_index($_REQUEST['thesaurus_term'], 'assetid', 0);
		$old_value		= array_get_index($_REQUEST, 'thesaurus_term_old', '');
		$root_node		= array_get_index($_REQUEST['root_node'], 'assetid', 0);
		$cascade		= array_get_index($_REQUEST, 'cascade', 0);

		if (empty($current_term) || empty($old_value)) {
			return FALSE;
		}//end if

		$term = $GLOBALS['SQ_SYSTEM']->am->getAsset($current_term, '', TRUE);
		if (is_null($term)) {
			return FALSE;
		}//end if

		// Locate the thesaurus
		$parts = explode(':', $term->id);
		$thesaurus_id = array_get_index($parts, 0, 0);
		if (empty($thesaurus_id)) {
			return FALSE;
		}//end if

		// Set the running vars for the HIPO job
		$new_value = $term->attr('name');
		$running_vars = Array(
							'thesaurusid'	=> $thesaurus_id,
							'terms'			=> Array($old_value => $new_value),
						);

		// If a root node is set, add it in
		if (!empty($root_node)) {
			$root = $GLOBALS['SQ_SYSTEM']->am->getAsset($root_node, '', TRUE);
			if (!is_null($root)) {
				$running_vars['assetids'] = Array($root->id);
				if (!empty($cascade)) {
					$running_vars['cascade'] = TRUE;
				}//end if
			}//end if
		}//end if

		// Run the hipo job
		$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$hh->queueHipo('hipo_job_rename_thesaurus_term', $running_vars);
		$url = $hh->runQueuedJobs();
		if (!empty($url)) $o->setRedirect($url);

		return TRUE;

	}//end processTool()


}//end class
?>
