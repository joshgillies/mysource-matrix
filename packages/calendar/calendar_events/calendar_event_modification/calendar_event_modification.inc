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
* $Id: calendar_event_modification.inc,v 1.33 2013/07/10 08:55:55 cupreti Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';
require_once  dirname(__FILE__).'/../calendar_event_recurring/calendar_event_recurring.inc';

/**
* Calendar_Event_Modification
*
* Objects of this class represent instances where the details (name or description)
* of a recurring event have changed for a particular instance of that event.
*
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.33 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Modification extends Calendar_Event
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Modification($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end constructor


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Modification events ensure their link data is correct and they are synchronized
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		$parent =& $link['asset'];
		if ($parent->type() != 'calendar_event_recurring') {
			trigger_localised_error('CAL0052', translate('Recurring Event Modifications can only be created underneath recurring events'), E_USER_ERROR);
			return FALSE;
		}

		if (!$this->synchronizeWithParent($parent)) {
			return FALSE;
		}

		if (!parent::_preCreateCheck($link)) return FALSE;

		if (!($link['value'] = $this->getStartDate())) {
			trigger_localised_error('CAL0049', translate('You need to specify a start date before you can call create() for a modification event'), E_USER_ERROR);
			return FALSE;
		}

		return TRUE;

	}//end _preCreateCheck()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	* @param int		$contextid	what context to return the name from
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE, $contextid=NULL)
	{
		// No context specified, using the current context
		if ($contextid === NULL) {
			$contextid = $GLOBALS['SQ_SYSTEM']->getContextId();
		}//end if

		if ($short_name) {
			return sprintf(translate('Modified %s'), date('d M Y', iso8601_ts($this->attr('start_date'))));

		}//end if

		// Obtain the attribute value for Name from the specified Context
		$values = $GLOBALS['SQ_SYSTEM']->am->getAttributeValuesByName('name', $this->type(), Array($this->id), $contextid);
		if (empty($values) === TRUE) {
			return parent::_getName($short_name, $contextid);
		} else {
			return $values[$this->id];
		}

	}//end _getName()


	/**
	* Check the parent has an instance on the specified date and set our end time, end date, end time and name based on the parent
	*
	* @param object	&$parent	The recurring event that we belong to
	*
	* @return boolean
	* @access public
	* @see Calendar_Event_Modification::create(), Calendar_Event_Modification::saveAttributes()
	*/
	function synchronizeWithParent(&$parent)
	{
		// check that it has an occurrence on the date specified
		if (!$parent->hasOccurrenceOnDate($this->getStartDate())) {
			trigger_localised_error('CAL0051', sprintf(translate('There is no occurrence of the specified recurring event on the specified date, being %s'), $this->getStartDate()), E_USER_WARNING);
			return FALSE;
		}

		// update our start time, end date and end time to correspond with the parent
		$sd = $this->getStartDate();
		$st = $parent->getStartTime();
		if ($st) {
			$this->setAttrValue('start_date', $sd.' '.$st.':00');
		}
		if ($duration = $parent->getDuration()) {
			$this->setAttrValue('end_date', date('Y-m-d', iso8601_ts($this->attr('start_date')) + $duration).substr($parent->attr('end_date'), 10));
		}

		// set our name based on the parent
		if (!$this->attr('name')) {
			$this->setAttrValue('name', $parent->attr('name').' - Modified '.date('d M Y', iso8601_ts($this->attr('start_date'))));
		}

		return TRUE;

	}//end synchronizeWithParent()


	/**
	* Save attributes set with setAttrValue to the database
	*
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run
	*										(THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	* @see Asset::setAttrValue()
	*/
	function saveAttributes($dont_run_updated=FALSE)
	{
		if ($this->id) {

			if (!$dont_run_updated) {

				// get the parent recurring event asset
				$recurring_event_ids = array_keys($GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE));
				if (empty($recurring_event_ids)) {
					trigger_localised_error('CAL0052', translate('Recurring Event Modifications can only be created underneath recurring events'), E_USER_ERROR);
					return FALSE;
				}
				$recurring_event =& $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_event_ids[0]);

				if (!$this->synchronizeWithParent($recurring_event)) {
					return FALSE;
				}
			}
		}
		return parent::saveAttributes($dont_run_updated);

	}//end saveAttributes()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$root_url = array_values($GLOBALS['SQ_SYSTEM']->am->getRootURL());
		if (is_array($root_url)) $root_url = $root_url[0];
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b><?php echo translate('Modification Of'); ?></b></td>

				<td><?php
				// get the parent recurring event asset
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE)) {
					$recurring_events = array_keys($recurring_events);
					$recurring_event =& $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					echo '<a href="'.$root_url.'?a='.$recurring_events[0].'">'.$recurring_event->attr('name').'</a>';
				}
				?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Start Date of modified instance'); ?></b></td>

				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('End Date of modified instance'); ?></b></td>

				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Modified Name'); ?></b></td>

				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Modified Description'); ?></b></td>

				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


	/**
	* Returns the start and end dates/times of this instance for use with
	* %instance_start...% and %instance_end...% keywords
	*
	* Return value is Array('start' => start date/time of instance,
	*						'end' => end date/time of instance)
	*
	* @param string	$date	The date to find an instance with
	*
	* @return array
	* @access public
	*/
	function getInstanceDates($date)
	{
		$res = Array();
		$res['start'] = $date;

		if ($this->getStartTime()) {
			$res['start'] .= ' '.$this->getStartTime();
		}

		$res['end'] = date('Y-m-d', iso8601_ts($res['start']) + $this->getDuration());

		if ($this->getEndTime()) {
			$res['end'] .= ' '.$this->getEndTime();
		}

		return $res;

	}//end getInstanceDates()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$full_keyword = $keyword;
		$keyword = parse_keyword($keyword, $modifiers);

		if (in_array($keyword, Array('instance_start_datetime', 'instance_end_datetime'))) {
			if (isset($_REQUEST['SQ_CALENDAR_DATE'])) {
				$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
				switch ($keyword) {
					case 'instance_start_datetime':
						$replacement = $instance_dates['start'];
					case 'instance_end_datetime':
						$replacement = $instance_dates['end'];
				}
			}
		}
		$matches = Array();
		preg_match('/^instance_start_datetime_(.+)/', $keyword, $matches);
		if ((!empty($matches)) && !empty($_REQUEST['SQ_CALENDAR_DATE'])) {
			$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
			$replacement = date($matches[1], iso8601_ts($instance_dates['start']));
		}
		$matches = Array();
		preg_match('/^instance_end_datetime_(.+)/', $keyword, $matches);
		if ((!empty($matches)) && !empty($_REQUEST['SQ_CALENDAR_DATE'])) {
			$instance_dates = $this->getInstanceDates($_REQUEST['SQ_CALENDAR_DATE']);
			$replacement = date($matches[1], iso8601_ts($instance_dates['end']));
		}

		if (empty($replacement)) {
			$replacement = parent::getKeywordReplacement($keyword);
		}

		if ($replacement != '%'.$keyword.'%') {
			apply_keyword_modifiers($replacement, $modifiers, Array('assetid' => $this->id));
		} else {
			$replacement = '%'.$full_keyword.'%';
		}

		return $replacement;

	}//end getKeywordReplacement()


	/**
	* Return the keywords for this asset type.
	*
	* This function is called in asset-listing situations like the asset listing, search page etc
	* The return value is in the form:
	* <PRE>
	* Array(
	*   'name' => 'description' => 'the asset name',
	* );
	* </PRE>
	*
	* @return array
	* @access public
	*/
	function getAvailableKeywords()
	{
		$res = parent::getAvailableKeywords();

		$our_keywords[] = 'instance_end_datetime';
		$our_keywords[] = 'instance_start_datetime';
		$our_keywords[] = 'instance_start_datetime_';
		$our_keywords[] = 'instance_end_datetime_';

		foreach ($our_keywords as $keyword) {
			$res[$keyword] = translate('cal_evt_recur_keyword_'.$keyword);
		}
		ksort($res);
		return $res;

	}//end getAvailableKeywords()


}//end class

?>
