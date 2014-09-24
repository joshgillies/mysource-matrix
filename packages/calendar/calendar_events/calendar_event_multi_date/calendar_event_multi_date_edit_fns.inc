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
* $Id: calendar_event_multi_date_edit_fns.inc,v 1.11 2012/08/30 00:57:28 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_PACKAGES_PATH.'/calendar/calendar_events/calendar_event_single/calendar_event_single_edit_fns.inc';
require_once SQ_PACKAGES_PATH.'/calendar/calendar_events/calendar_event_recurring/calendar_event_recurring_edit_fns.inc';


/**
* Multi-Date Event Edit Functions
*
* @author  Greg Sherwoood <greg@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.11 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Multi_Date_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Multi_Date_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->_tmp['single_edit_fns'] = new Calendar_Event_Single_Edit_Fns();
		$this->_tmp['rec_edit_fns'] = new Calendar_Event_Recurring_Edit_Fns();

	}//end constructor


	/**
	* Prints the interface for choosing dates for this event
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDates(&$asset, &$o, $prefix)
	{
		$wa = $asset->writeAccess('attributes');
		?>
		<style type="text/css">
			td.single table td, td.recurring table td {
				border: 0px;
				padding: 1px;
			}
		</style>

		<table border="1" cellspacing="0" cellpadding="4" class="sq-backend-table">
			<thead>
				<tr>
					<th>Type</th>
					<th>Details</th>
				<?php
				if ($wa) {
					?>
					<th>Delete</th>
					<?php
				}
				?>
				</tr>
			</thead>
			<tbody>
				<?php
				if ($asset->id) {
					$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('calendar_event_single', 'calendar_event_recurring'));
					foreach ($children as $id => $type) {
						$_type = $type[0]['type_code'];
						$event_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $_type);
						$this->_printDateRow($prefix, $event_asset, $o, $wa);
					}
				}
				if ($wa) $this->_printDateTemplate($prefix, $o);
				?>
			</tbody>
		</table>
		<?php
		if ($wa) {
			?>
			<script type="text/javascript" src="<?php echo sq_web_path('data'); ?>/asset_types/calendar_event_multi_date/js/dates_chooser.js"></script>
			<?php
			normal_button('', 'More Dates...', 'addDate()');
		}

	}//end paintDates()


	/**
	* Paint a row in the instances table for the supplied event
	*
	* @param string		$prefix			The prefix for form fields
	* @param object		$event_asset	The event asset of the instance being represented
	* @param object		$o				The backend outputter we are using
	* @param boolean	$write_access	Whether we have write access to the instance
	*
	* @return void
	* @access protected
	*/
	function _printDateRow($prefix, $event_asset, $o, $write_access)
	{
		if ($write_access) {
			$GLOBALS['SQ_SYSTEM']->am->acquireLock($event_asset->id, 'attributes');
		} else {
			$GLOBALS['SQ_SYSTEM']->am->releaseLock($event_asset->id, 'attributes');
		}
		$type_name = str_replace('calendar_event_', '', $event_asset->type());
		if (is_a($event_asset, 'calendar_event_single') || is_a($event_asset, 'calendar_event_recurring')) {
		?>
			<tr>
				<td>
					<a href="<?php echo $event_asset->getBackendHref('details'); ?>">
						<?php echo ucfirst($type_name); ?> Instance #<?php echo $event_asset->id; ?>
					</a>
				</td>
				<td class="<?php echo $type_name; ?>">
					<?php
					if (is_a($event_asset, 'calendar_event_single')) {
						$this->_tmp['single_edit_fns']->paintSingleStartEndChooser($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
					} else if (is_a($event_asset, 'calendar_event_recurring')) {
						// recurring
						$this->_tmp['rec_edit_fns']->paintRecurringStartEndChooser($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
						if ($write_access) {
							echo '<br />Recurs<br />';
							$this->_tmp['rec_edit_fns']->paintFrequencyField($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
						} else {
							echo ', and then ';
							ob_start();
							$this->_tmp['rec_edit_fns']->paintFrequencyField($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
							$res = ob_get_clean();
							echo strtolower($res{0}).substr($res, 1); // I want lcfirst!!
						}
					}
					?>
				</td>
				<td style="display: none"></td>
				<?php
				if ($write_access) {
					?>
					<td><?php check_box($prefix.'_existing_'.$event_asset->id.'_delete'); ?></td>
					<?php
				}
				?>
			</tr>
		<?php
		}//end if

	}//end _printDateRow()


	/**
	* Print the hidden template row in the instances table, used for adding new instances
	*
	* @param string	$prefix	The prefix for form fields
	* @param object	$o		The backend outputter we are using
	*
	* @return void
	* @access protected
	*/
	function _printDateTemplate($prefix, $o)
	{
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_recurring');
		$fake_rec_event = new Calendar_Event_Recurring();
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_single');
		$fake_single_event = new Calendar_Event_Single();
		?>
			<tr id="template" style="display: none">
				<td style="white-space: nowrap; vertical-align: top">
					<?php
					radio_button($prefix.'_new_type_template', 'single', TRUE, 'setEventType(this)', 'id="'.$prefix.'_new_type_template_single"');
					label('Single Instance', $prefix.'_new_type_template_single');
					echo '<br />';
					radio_button($prefix.'_new_type_template', 'recurring', FALSE, 'setEventType(this)', 'id="'.$prefix.'_new_type_template_recurring"');
					label('Recurring Instance', $prefix.'_new_type_template_recurring');
					echo '<br />';
					?>
				</td>
				<td class="single">
					<?php
					$this->_tmp['single_edit_fns']->paintSingleStartEndChooser($fake_single_event, $o, $prefix.'_new_single_template');
					?>
				</td>
				<td class="recurring" style="display: none">
					<?php
					$this->_tmp['rec_edit_fns']->paintRecurringStartEndChooser($fake_rec_event, $o, $prefix.'_new_recurring_template');
					echo '<br />Recurs<br />';
					$this->_tmp['rec_edit_fns']->paintFrequencyField($fake_rec_event, $o, $prefix.'_new_recurring_template');
					?>
				</td>
				<td>
					<?php sq_print_icon(sq_web_path('lib').'/web/images/icons/delete.png', 16, 16, 'Delete New Row', 'Delete New Row', 'onclick="if (confirm(\'Delete new instance?\')) deleteRow(this)" style="cursor: pointer"'); ?>
				</td>
			</tr>
		<?php

	}//end _printDateTemplate()


	/**
	* Process the interface for choosing dates for this event
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processDates(&$asset, &$o, $prefix)
	{
		if ($asset->id) {
			// Existing instances
			if (!$asset->writeAccess('attributes')) return FALSE;
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($asset->id, Array('calendar_event_single', 'calendar_event_recurring'));
			foreach ($children as $id => $type) {

				if (strpos($id, ':') !== FALSE) continue;

				if (isset($_POST[$prefix.'_existing_'.$id.'_delete'])) {
					if ($GLOBALS['SQ_SYSTEM']->am->trashAsset($id)) {
						$instance = $GLOBALS['SQ_SYSTEM']->am->getAsset($id);
						$instance->saveWebPaths(Array(), FALSE);
					}
				} else {
					$type = $type[0]['type_code'];
					$event_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($id, $type);
					$need_save = FALSE;
					if ($type == 'calendar_event_single') {
						$need_save |= $this->_tmp['single_edit_fns']->processSingleStartEndChooser($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
					} else {
						// recurring event
						$need_save |= $this->_tmp['rec_edit_fns']->processRecurringStartEndChooser($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
						$need_save |= $this->_tmp['rec_edit_fns']->processFrequencyField($event_asset, $o, $prefix.'_existing_'.$event_asset->id);
					}
					if ($need_save) {
						$GLOBALS['SQ_SYSTEM']->am->acquireLock($event_asset->id, 'attributes');
						$event_asset->saveAttributes();
					}
					$GLOBALS['SQ_SYSTEM']->am->forgetAsset($id);
				}
			}
		}

		//if we are going to manipulate asset while is safe edit, let us first save its links beforehand
		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			if (!file_exists($asset->data_path.'/.safe_edit_before')) {
				$child_events = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, Array('calendar_event_single', 'calendar_event_recurring'), FALSE);
				file_put_contents($asset->data_path.'/.safe_edit_before', serialize($child_events));
			}
		}

		// New instances
		$i = 0;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_single');
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('calendar_event_recurring');
		while (isset($_POST[$prefix.'_new_type_'.$i])) {
			if (!empty($_POST[$prefix.'_new_type_'.$i])) {
				switch ($_POST[$prefix.'_new_type_'.$i]) {
					case 'single':
						$new_asset = new Calendar_Event_Single();
						$this->_tmp['single_edit_fns']->processSingleStartEndChooser($new_asset, $o, $prefix.'_new_single_'.$i);
					break;

					case 'recurring':
						$new_asset = new Calendar_Event_Recurring();
						$this->_tmp['rec_edit_fns']->processRecurringStartEndChooser($new_asset, $o, $prefix.'_new_recurring_'.$i);
						$this->_tmp['rec_edit_fns']->processFrequencyField($new_asset, $o, $prefix.'_new_recurring_'.$i);
					break;
				}
				$new_asset->setAttrValue('name', $asset->attr('name'));
				$link = Array(
							'asset'			=> &$asset,
							'link_type'		=> SQ_LINK_TYPE_3,
							'value'			=> '',
							'sort_order'	=> -1,
							'is_dependant'	=> TRUE,
							'is_exclusive'	=> TRUE,
						);
				if ($asset->id) {
					$new_asset->create($link);
					//we are done creating asset instance, lets make sure bug # 4168 doesnt occur
					if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
						if ($new_asset->status != SQ_STATUS_EDITING) {
							// Drop to a run level to force the status change to SQ_STATUS_EDITING (saveSystemVersion() so it can go back to LIVE)
							$GLOBALS['SQ_SYSTEM']->setRunLevel(191);
							$new_asset->saveSystemVersion();
							$new_asset->processStatusChange(SQ_STATUS_EDITING, FALSE);
							$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
						}
					}
				} else {
					// line it up for creation after the parent asset is created
					$asset->addPendingChild($new_asset, $link);
				}
			}//end if
			$i++;
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_asset);
		}//end while

		//adding all the new instances, save the current links to file
		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING){
				$child_events = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_3, Array('calendar_event_single', 'calendar_event_recurring'), FALSE);
				file_put_contents($asset->data_path.'/.safe_edit_after', serialize($child_events));
		}

	}//end processDates()


	/**
	* Paint the interface for creating tables in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function redirectToBodycopy(&$asset, &$o, $prefix)
	{
		// get the bodycopy attached to us
		$bodycopy_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', NULL, '1', '0');
		$bodycopy = $GLOBALS['SQ_SYSTEM']->am->getAsset($bodycopy_link[0]['minorid'], 'bodycopy');

		if (is_null($bodycopy)) {
			trigger_localised_error('CORE0016', sprintf(translate('Unable to locate the bodycopy for standard page "%s"'), $asset->name), E_USER_ERROR);
			return FALSE;
		}

		// redirect to the bodycopy's edit contents screen
		$o->setRedirect($bodycopy->getBackendHref('contents'));

		// fudge the navigation history a little so when they click on the link to ger back to the
		// page it doesnt take them to the edit contents screen and then redirect them again
		$_SESSION['sq_nav_history'][(count($_SESSION['sq_nav_history']) - 1)]['screen'] = 'details';

		return FALSE;

	}//end redirectToBodycopy()


}//end class

?>
