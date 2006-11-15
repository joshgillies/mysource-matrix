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
* $Id: search_manager_prefs.inc,v 1.1 2006/11/15 04:11:14 lwright Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Search_Manager_Prefs
*
* Purpose
*
*    Looks after the creation the Search Manager preferences
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix
*/
class Search_Manager_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_SEARCH_BACKEND_PAGE_SIZE'	=> Array(
												'name'			=> 'Backend Search Page Size',
												'description'	=> 'Set how many results are displayed to a page when a keyword search is performed from the Quick Search box',
												'default'		=> 5,
												'protected'		=> FALSE,
											   ),
					 );


	/**
	* Constructor
	*
	*/
	function Content_Type_Raw_HTML_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (is_null($pref) || $pref == 'SQ_SEARCH_BACKEND_PAGE_SIZE') {
			if (is_null($pref)) {
				$o->openField(translate('sch_pref_backend_page_size'));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_SEARCH_BACKEND_PAGE_SIZE]', $this->pref_vars['SQ_SEARCH_BACKEND_PAGE_SIZE']['default'], 10);
			} else {
				echo array_get_index($this->pref_vars['SQ_SEARCH_BACKEND_PAGE_SIZE'], 'default', 5);
			}
			echo ' '.translate('asset(s)');

			echo $o->note(translate('sch_pref_backend_page_size_note'));
			if (is_null($pref)) $o->closeField();
		}//end if

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=NULL)
	{
		// Not set = just silently return that we haven't done anything
		if (!isset($_POST['prefs']['SQ_SEARCH_BACKEND_PAGE_SIZE'])) {
			return FALSE;
		}

		// Not an integer = user mistake, or possible script injection?
		if (is_int($_POST['prefs']['SQ_SEARCH_BACKEND_PAGE_SIZE'])) {
			trigger_localised_error('SCH0029', E_USER_WARNING);
			return FALSE;
		}

		$page_size = (int)$_POST['prefs']['SQ_SEARCH_BACKEND_PAGE_SIZE'];

		// Page size needs to be at least 1
		if ($page_size <= 0) {
			trigger_localised_error('SCH0029', E_USER_WARNING);
			return FALSE;
		}

		$this->pref_vars['SQ_SEARCH_BACKEND_PAGE_SIZE']['default'] = $page_size;
		return TRUE;

	}//end processBackend()


}//end class

?>