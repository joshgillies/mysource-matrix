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
* $Id: calendar_event_recurring_prefs.inc,v 1.3 2012/08/30 00:57:28 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/config/prefs.inc';


/**
* Calendar_Event_Recurring_Prefs
*
* Purpose
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: 1.3 $
* @package MySource_Matrix
*/
class Calendar_Event_Recurring_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_CALENDAR_EVENTS_FREQUENCY'	=> Array(
														'name'			=> 'calendar_events_frequency',
														'description'	=> 'Recurrence event frequency threshold in days. If set to a non-zero positive value, the recurring event will be displayed only once in listing pages when its frequency is less than the specified threshold value.',
														'default'		=> 0,
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

		if (is_null($pref) || $pref == 'SQ_CALENDAR_EVENTS_FREQUENCY') {
			if (is_null($pref)) {
				$o->openField(translate('Events frequency threshold'));
			}

			if ($have_lock && $is_admin) {
				text_box('prefs[SQ_CALENDAR_EVENTS_FREQUENCY]', $this->pref_vars['SQ_CALENDAR_EVENTS_FREQUENCY']['default'], 5);
			} else {
				echo array_get_index($this->pref_vars['SQ_CALENDAR_EVENTS_FREQUENCY'], 'default', 5);
			}

			 echo '<div class="sq-backend-note">'.translate('Recurrence event frequency threshold in days. If set to a non-zero positive value, the recurring event will be displayed only once in listing pages when its frequency is less than the specified threshold value.').'</div>';

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
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		// Backend page size
		if (isset($_POST['prefs']['SQ_CALENDAR_EVENTS_FREQUENCY'])) {

			// Not an integer = user mistake, or possible script injection?
			if (!is_numeric($_POST['prefs']['SQ_CALENDAR_EVENTS_FREQUENCY'])) {
				trigger_error('Recurring events frequency threshold must be an integer', E_USER_WARNING);
				return FALSE;
			}
			$frequency_threshold = (int)$_POST['prefs']['SQ_CALENDAR_EVENTS_FREQUENCY'];

			// Frequency threshold shouldnt be than zero
			if ($frequency_threshold < 0) {
				trigger_error('Recurring events frequency threshold must be an integer', E_USER_WARNING);
				return FALSE;
			}

			$this->pref_vars['SQ_CALENDAR_EVENTS_FREQUENCY']['default'] = $frequency_threshold;
		}

		return TRUE;

	}//end processBackend()


}//end class

?>
