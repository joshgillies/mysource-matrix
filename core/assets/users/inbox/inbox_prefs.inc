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
* $Id: inbox_prefs.inc,v 1.4 2012/08/30 01:09:16 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';

/**
* Inbox_Manager_Prefs
*
* Purpose
*
*    Looks after the creation the Inbox preferences
*
* @author  Christophe Olivar <colivar@squiz.net>
* @version $Revision: 1.4 $
* @package MySource_Matrix
*/
class Inbox_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_INBOX_MAIL_PER_PAGE'	=> Array(
														'default'		=> 20,
														'protected'		=> FALSE,
														),
					);


	/**
	* Constructor
	*
	*/
	function Inbox_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);

		$this->pref_vars['SQ_INBOX_MAIL_PER_PAGE']['name'] = translate('Messages per page');

	}//end inboxPrefs()


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

	    // Messages per page.
		$this->paintTextField($o, $have_lock, $pref, 'SQ_INBOX_MAIL_PER_PAGE', 5, '%s message(s)');

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

		/*		$changed = FALSE; */

		// Backend page size
		if (isset($_POST['prefs']['SQ_INBOX_MAIL_PER_PAGE'])) {

			// Not an integer = user mistake, or possible script injection?
			if (!is_numeric($_POST['prefs']['SQ_INBOX_MAIL_PER_PAGE'])) {
				trigger_localised_error('IBOX0001', translate('The Number of emails per page must be an integer and at least 1'), E_USER_WARNING);
				return FALSE;
			}
			$mail_number = (int)$_POST['prefs']['SQ_INBOX_MAIL_PER_PAGE'];

			// Page size needs to be at least 1
			if ($mail_number <= 0) {
				trigger_localised_error('IBOX0001', translate('The Number of emails per page must be an integer and at least 1'), E_USER_WARNING);
				return FALSE;
			}

			$this->pref_vars['SQ_INBOX_MAIL_PER_PAGE']['default'] = $mail_number;

			/*			$changed = TRUE; */
		}


		return TRUE;

	}//end processBackend()


}//end class

?>
