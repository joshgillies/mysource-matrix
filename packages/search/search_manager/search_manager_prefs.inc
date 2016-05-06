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
* $Id: search_manager_prefs.inc,v 1.7 2012/08/30 00:59:15 ewang Exp $
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
* @version $Revision: 1.7 $
* @package MySource_Matrix
*/
class Search_Manager_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_SEARCH_BACKEND_PAGE_SIZE'		=> Array(
																'default'		=> 5,
																'protected'		=> FALSE,
															   ),
						'SQ_SEARCH_BACKEND_RESULT_LIMIT'	=> Array(
																'default'		=> 100,
																'protected'		=> FALSE,
															   ),
					 );


	/**
	* Constructor
	*
	*/
	function __construct($pref_file='')
	{
		parent::__construct($pref_file);

		$this->pref_vars['SQ_SEARCH_BACKEND_PAGE_SIZE']['name'] = translate('Backend Search Page Size');
		$this->pref_vars['SQ_SEARCH_BACKEND_PAGE_SIZE']['desc'] = translate('Sets how many assets will be shown per page when a keyword search is peformed through the Quick Search box.');

		$this->pref_vars['SQ_SEARCH_BACKEND_RESULT_LIMIT']['name'] = translate('Backend Search Result Limit');
		$this->pref_vars['SQ_SEARCH_BACKEND_RESULT_LIMIT']['desc'] = translate('The maximum number of results displayed in backend search (0 = no limit).');

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
	    $is_admin  = ($GLOBALS['SQ_SYSTEM']->userRoot() || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
	    $have_lock = $have_lock && $is_admin;

	    // Assets per page.
		$this->paintIntTextField($o, $have_lock, $pref, 'SQ_SEARCH_BACKEND_PAGE_SIZE', 5, translate('%s asset(s)'), 0);
		$this->paintIntTextField($o, $have_lock, $pref, 'SQ_SEARCH_BACKEND_RESULT_LIMIT', 5, function($value) use ($have_lock)  {
			if ($have_lock) {
				return $value;
			} else {
				if ((int) $value === 0) {
					return translate('Unlimited');
				} else {
					return sprintf(translate('%s asset(s)'), $value);
				}
			}
		}, 0);

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
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		$changed = FALSE;

		// Backend page size
		if (isset($_POST['prefs']['SQ_SEARCH_BACKEND_PAGE_SIZE'])) {

			// Not an integer = user mistake, or possible script injection?
			if (!is_numeric($_POST['prefs']['SQ_SEARCH_BACKEND_PAGE_SIZE'])) {
				trigger_localised_error('SCH0029', translate('The Backend Search Page Size must be an integer and at least 1'), E_USER_WARNING);
				return FALSE;
			}

			$page_size = (int)$_POST['prefs']['SQ_SEARCH_BACKEND_PAGE_SIZE'];

			// Page size needs to be at least 1
			if ($page_size <= 0) {
				trigger_localised_error('SCH0029', translate('The Backend Search Page Size must be an integer and at least 1'), E_USER_WARNING);
				return FALSE;
			}

			$this->pref_vars['SQ_SEARCH_BACKEND_PAGE_SIZE']['default'] = $page_size;

			$changed = TRUE;
		}

		if (isset($_POST['prefs']['SQ_SEARCH_BACKEND_RESULT_LIMIT'])) {

			// Not an integer = user mistake, or possible script injection?
			if (!is_numeric($_POST['prefs']['SQ_SEARCH_BACKEND_RESULT_LIMIT'])) {
				trigger_localised_error('SCH0030', translate('The Backend Search Result Limit must be an integer and at least 0'), E_USER_WARNING);
				return FALSE;
			}

			$result_limit = (int)$_POST['prefs']['SQ_SEARCH_BACKEND_RESULT_LIMIT'];

			// Page size needs to be at least 1
			if ($result_limit < 0) {
				trigger_localised_error('SCH0030', translate('The Backend Search Result Limit must be an integer and at least 0'), E_USER_WARNING);
				return FALSE;
			}

			$this->pref_vars['SQ_SEARCH_BACKEND_RESULT_LIMIT']['default'] = $result_limit;

			$changed = TRUE;
		}

		return TRUE;

	}//end processBackend()


}//end class

?>
