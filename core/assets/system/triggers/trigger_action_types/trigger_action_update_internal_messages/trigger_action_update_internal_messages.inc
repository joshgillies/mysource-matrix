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
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/triggers/trigger_action/trigger_action.inc';
require_once SQ_SYSTEM_ROOT.'/core/attributes/parameter_map/parameter_map.inc';
require_once SQ_SYSTEM_ROOT.'/core/include/messaging_service.inc';


/**
* Trigger_Action_Update_Internal_Messages
*
* Purpose
* 	Trigger Action to update the internal message(s) attributes
*
* The settings used by this action are in the form:<BR />
* <PRE>
* 	Array(
*       'status'	=> string // action to do for the given message(s) to
*       'source'	=> string // source to get the messages from
*       'value_map'	=> array // message id(s) source
*       );
* </PRE>
*
* @author  Chiranjivi Upreti <cupreti@squiz.com.au>
* @version $Revision: $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Trigger_Action_Update_Internal_Messages extends Trigger_Action
{


	static private $_statii = Array(
							'read' => 'Mark as read',
							'unread' => 'Mark as unread',
							'trash' => 'Move to trash',
							'trash_rr' => 'Recover from trash (and mark as read)',
							'trash_ru' => 'Recover from trash (and mark as unread)',
							'purge' => 'Purge from trash',
						);
	static private $_msg_source = Array(
							'inbox' => 'All messages in the user\'s inbox',
							'sent' => 'All messages in the user\'s sent folder',
							'trash' => 'All messages in the user\'s trash',
							'dynamic' => 'Dynamic source',
						);


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Execute this action
	* Returns an array of data about what it did, or false on error
	*
	* The settings used by this action are in the form:<BR />
	* <PRE>
	* Array(
	*       'status'	=> string // action to do for the given message(s) to
	*       'source'	=> string // source to get the messages from
	*       'value_map'	=> array // message id(s) source
	*       );
	* </PRE>
	*
	* @param array	$settings	the stored settings for this action
	* @param array	&$state		the state that describes the asset the action is executing on
	*
	* @return mixed array|boolean
	* @access public
	*/
	public static function execute($settings, &$state)
	{
		$message_action = array_get_index($settings, 'status', SQ_MSG_READ);
		if (!in_array($message_action, array_keys(self::$_statii))) {
			trigger_localised_error('CORE0322', sprintf(translate('Invalid message action "%1$s".'), $message_action), E_USER_WARNING)
			return FALSE;
		}

		$message_source = array_get_index($settings, 'source', SQ_MSG_READ);
		if (!in_array($message_source, array_keys(self::$_msg_source))) {
			trigger_localised_error('CORE0323', sprintf(translate('Invalid message source "%1$s".'), $message_source), E_USER_WARNING)
			return FALSE;
		}

		// Whether the action is specific to messages in trash only
		$trash_specific = in_array($message_action, Array('trash_rr', 'trash_ru', 'purge'));

		$ms = $GLOBALS['SQ_SYSTEM']->getMessagingService();
		$current_user = !is_null($GLOBALS['SQ_SYSTEM']->user) ? $GLOBALS['SQ_SYSTEM']->user : NULL;

		$raw_message_ids = Array();
		// Get the message ids from the source
		switch($message_source) {
			case 'inbox':
				if (!$trash_specific && $current_user) {
					$raw_message_ids = $ms->getInternalMessages($current_user->id, Array(SQ_MSG_UNREAD, SQ_MSG_READ), '-1', NULL, Array(), NULL, NULL, TRUE);
				}
			break;

			case 'sent':
				if (!$trash_specific && $current_user) {
					$raw_message_ids = $ms->getInternalMessages($current_user->id, Array(SQ_MSG_UNREAD, SQ_MSG_READ), '-1', 'inbox.sent', Array(), NULL, NULL, TRUE);
				}
			break;

			case 'trash':
				if ($trash_specific && $current_user) {
					$raw_message_ids = $ms->getInternalMessages($current_user->id, Array(SQ_MSG_DELETED), '-1', NULL, Array(), NULL, NULL, TRUE);
				}
			break;

			case 'dynamic':
				$old_current_asset = $GLOBALS['SQ_SYSTEM']->getGlobalDefine('CURRENT_ASSET', NULL);
				$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $state['asset']);

				$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
				$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);
				// Get the message ids to action on from the parameter map
				$value_map_value = $atr_parameter_map->getParameterValue('message_ids');
				$raw_message_ids = explode(',', $value_map_value);

				if (is_null($old_current_asset)) {
					$GLOBALS['SQ_SYSTEM']->unsetGlobalDefine('CURRENT_ASSET');
				} else {
					$GLOBALS['SQ_SYSTEM']->setGlobalDefine('CURRENT_ASSET', $old_current_asset);
				}
			break;
		}

		$message_ids = Array();
		if (!empty($raw_message_ids)) {
			foreach($raw_message_ids as $index => $message_id) {
				$message_id = trim($message_id);
				if (empty($message_id) || !ctype_digit((string) $message_id)) {
					continue;
				}
				$message_ids[$message_id] = MatrixDAL::quote($message_id);
			}//end foreach

			if (!empty($message_ids)) {

				// Break up the assets into chunks of 1000 so that oracle does not complain
				$in_clauses = Array();
				foreach(array_chunk($message_ids, 999) as $chunk) {
					$in_clauses[] = ' msgid IN ('.implode(', ', $chunk).')';
				}
				$where = '('.implode(' OR ', $in_clauses).') AND ';

				// Message action
				switch($message_action) {
					case 'read':
						// Mark inbox/sent messags as "read"
						$new_status = SQ_MSG_READ;
						$where .= 'status <> \''.SQ_MSG_DELETED.'\'';
					break;
					case 'unread':
						// Mark inbox/sent messags as "unread"
						$new_status = SQ_MSG_UNREAD;
						$where .= 'status <> \''.SQ_MSG_DELETED.'\'';
					break;
					case 'trash':
						// Mark inbox/sent messages as "trashed"
						$new_status = SQ_MSG_DELETED;
						$where .= 'status <> \''.SQ_MSG_DELETED.'\'';
					break;
					case 'trash_rr':
						// Recover messages from trash and mark it as "read"
						$new_status = SQ_MSG_READ;
						$where .= 'status = \''.SQ_MSG_DELETED.'\'';
					break;
					case 'trash_ru':
						// Recover messages from trash and mark it as "read"
						$new_status = SQ_MSG_UNREAD;
						$where .= 'status = \''.SQ_MSG_DELETED.'\'';
					break;
					case 'purge':
						// Purge messages from trash and mark it as "read"
						$where .= 'status = \''.SQ_MSG_DELETED.'\'';
					break;
				}//end switch

				if ($message_action == 'purge') {
					$sql = 'DELETE FROM
								sq_internal_msg
							WHERE '.$where;
				} else {
					$sql = 'UPDATE
							sq_internal_msg
						SET
							status = '.MatrixDAL::quote($new_status).'
						WHERE '.$where;
				}

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				try {
					$query = MatrixDAL::preparePdoQuery($sql);
					$ret = MatrixDAL::execPdoQuery($query);
				} catch (Exception $e) {
					 throw new Exception('Unable to update the internal message data for message id(s) "'.implode(',', $message_ids).', due to database error: '.$e->getMessage());
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');

			}//end if
		}//end if

		return Array(
				'updated_message_ids'	=> array_keys($message_ids),
			   );

	}//end execute()


	/**
	* Get the HTML editing interface for this action. Returns the string instead of printing it
	*
	* @param array		$settings		settings that this condition saves in processInterface()
	* @param string		$prefix			unique prefix
	* @param boolean	$write_access	determines whether the interface is editable
	*
	* @return boolean
	* @access public
	*/
	public static function getInterface($settings, $prefix, $write_access=FALSE)
	{
		$new_prefix = str_replace(array('[',']'), '_', $prefix);

		// Message status/action
		$default_status =  array_shift(array_keys(self::$_statii));
		$current_status = array_get_index($settings, 'status', $default_status);
		ob_start();
			combo_box($prefix.'[status]', self::$_statii, FALSE, $current_status, 0, (!$write_access ? ' disabled' : ''));
		$source_input = ob_get_clean();

		// Message source
		$default_source =  array_shift(array_keys(self::$_msg_source));
		$current_source = array_get_index($settings, 'source', $default_source);
		ob_start();
			combo_box($prefix.'[source]', self::$_msg_source, FALSE, $current_source, 0, 'onChange="'.$new_prefix.'_toggleParameterMap()"'.(!$write_access ? ' disabled' : ''));
		$status_input = ob_get_clean();

		ob_start();
			hidden_field($prefix.'[new_prefix]', $new_prefix);

			$parameter_map_value = array_get_index($settings, 'value_map', serialize(Array()));
			$atr_parameter_map = new Asset_Attribute_Parameter_Map(0, $parameter_map_value);

			$atr_parameter_map->setParameter('message_ids', translate('Comma separated list of message id(s)'));

			echo $atr_parameter_map->paint($new_prefix.'_parameter_map', !$write_access);
		$parameter_map_input = ob_get_clean();

		ob_start();
		?>
			<div>
				<table class="sq-backend-table" border="0">
					<tr>
						<th width="20%"><?php echo translate('Message action'); ?></th>

						<td><?php echo $source_input; ?></td>
					</tr>
					<tr>
						<th width="20%"><?php echo translate('Message source'); ?></th>

						<td><?php echo $status_input; ?>
							<div id="<?php echo $new_prefix ?>_dynamic_source">
								<br/>
								<?php echo $parameter_map_input; ?>
							</div>
						</td>
					</tr>
				</table>
				<script>
					function <?php echo $new_prefix; ?>_toggleParameterMap()
					{
						if (document.getElementById('<?php echo $prefix; ?>[source]').value == 'dynamic') {
							document.getElementById('<?php echo $new_prefix ?>_dynamic_source').style.display = "block";
						} else {
							document.getElementById('<?php echo $new_prefix ?>_dynamic_source').style.display = "none";
						}
					}
					<?php echo $new_prefix; ?>_toggleParameterMap();
				</script>
			</div>

		<?php
		$output = ob_get_clean();

		return $output;

	}//end getInterface()


	/**
	* Function that handles the conversion of interface to settings
	* together with settings it is expected to populate the hash object
	*
	* @param array	&$settings		a container for any data the action might want to save
	* @param string	$request_data	array of data corresponding to this action as
	*								specified by the $prefix in getInterface.
	*
	* @return boolean
	* @access public
	*/
	public static function processInterface(&$settings, $request_data)
	{
		$default_status =  array_shift(array_keys(self::$_statii));
		$settings['status'] = array_get_index($request_data, 'status', $default_status);

		$default_source =  array_shift(array_keys(self::$_msg_source));
		$settings['source'] = array_get_index($request_data, 'source', $default_source);

		$settings['value_map'] = '';
		$new_prefix = str_replace(array('[',']'), '_', array_get_index($request_data, 'new_prefix', ''));
		if ($new_prefix) {
			$atr_parameter_map = new Asset_Attribute_Parameter_Map();
			$atr_parameter_map->process($new_prefix.'_parameter_map');
			$settings['value_map'] = $atr_parameter_map->value;
		}

		return FALSE;

	}//end processInterface()


}//end class

?>
