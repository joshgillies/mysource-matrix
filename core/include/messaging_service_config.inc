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
* $Id: messaging_service_config.inc,v 1.23 2006/04/18 10:32:49 gnoel Exp $
*
*/


require_once SQ_LIB_PATH.'/config/config.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Messaging_Service_Config
*
* Purpose
*
*    Looks after the creation and editing of the messaging service config
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix
*/
class Messaging_Service_Config extends Config
{

	/**
	* The array of default messaging config settings
	* @var array
	*/
	var $config_vars = Array(
						'SQ_MS_LOG_TO_FILE_WHITE_LIST'	=> Array('editable' => 1, 'default' => '*'),
						'SQ_MS_LOG_TO_FILE_BLACK_LIST'	=> Array('editable' => 1, 'default' => "asset.attributes\nasset.workflow.invitation\nasset.workflow.announce\nasset.workflow.invitation.*\nasset.workflow.announce.*"),
						'SQ_MS_LOG_TO_DB_WHITE_LIST'	=> Array('editable' => 1, 'default' => '*'),
						'SQ_MS_LOG_TO_DB_BLACK_LIST'	=> Array('editable' => 1, 'default' => "asset.attributes.fulllog\nasset.workflow.invitation\nasset.workflow.announce\nasset.workflow.invitation.*\nasset.workflow.announce.*\ninstall.*"),
						'SQ_MS_SEND_MAIL_WHITE_LIST'	=> Array('editable' => 1, 'default' => "asset.locking.forced\nconfig.*\nhipo.*\ninbox.*\nasset.status.notify\nasset.workflow.invitation\nasset.workflow.announce\nasset.workflow.invitation.*\nasset.workflow.announce.*\nasset.workflow.stale\ncron.deadlock"),
						'SQ_MS_SEND_MAIL_BLACK_LIST'	=> Array('editable' => 1, 'default' => ''),
					   );


	/**
	* Constructor
	*
	*/
	function Messaging_Service_Config()
	{
		$this->Config();
		$this->config_file = SQ_DATA_PATH.'/private/conf/messaging_service.inc';

	}//end constructor


	/**
	* Paints the interface for our config options.
	*
	* If you (as a sub-class) put your config vars so that when they are submitted appear in
	* $_POST[get_class($this)] then you probably won't need to override processBackend()
	*
	* @param object	&$o	Backend_Outputter, reference to the backend outputter
	*
	* @return void
	* @access public
	* @see processBackend()
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

		$write_access = $this->writeAccess();
		$class = get_class($this);

		$o->openSection(translate('hipo_server_configuration'));
			$o->openField(translate('messaging_service_file_white_list'));
				if ($write_access) {
					text_area($class.'[SQ_MS_LOG_TO_FILE_WHITE_LIST]', SQ_MS_LOG_TO_FILE_WHITE_LIST, 60, 10);
				} else {
					echo nl2br(SQ_MS_LOG_TO_FILE_WHITE_LIST);
				}
				$o->note(translate('messaging_service_file_white_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_file_black_list'));
				if ($write_access) {
					text_area($class.'[SQ_MS_LOG_TO_FILE_BLACK_LIST]', SQ_MS_LOG_TO_FILE_BLACK_LIST, 60, 10);
				} else {
					echo nl2br(SQ_MS_LOG_TO_FILE_BLACK_LIST);
				}
				$o->note(translate('messaging_service_file_black_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_db_white_list'));
				if ($write_access) {
					text_area($class.'[SQ_MS_LOG_TO_DB_WHITE_LIST]', SQ_MS_LOG_TO_DB_WHITE_LIST, 60, 10);
				} else {
					echo nl2br(SQ_MS_LOG_TO_DB_WHITE_LIST);
				}
				$o->note(translate('messaging_service_db_white_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_db_black_list'));
				if ($write_access) {
					text_area($class.'[SQ_MS_LOG_TO_DB_BLACK_LIST]', SQ_MS_LOG_TO_DB_BLACK_LIST, 60, 10);
				} else {
					echo nl2br(SQ_MS_LOG_TO_DB_BLACK_LIST);
				}
				$o->note(translate('messaging_service_db_black_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_mail_white_list'));
				if ($write_access) {
					text_area($class.'[SQ_MS_SEND_MAIL_WHITE_LIST]', SQ_MS_SEND_MAIL_WHITE_LIST, 60, 10);
				} else {
					echo nl2br(SQ_MS_SEND_MAIL_WHITE_LIST);
				}
				$o->note(translate('messaging_service_mail_white_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_mail_black_list'));
				if ($write_access) {
					text_area($class.'[SQ_MS_SEND_MAIL_BLACK_LIST]', SQ_MS_SEND_MAIL_BLACK_LIST, 60, 10);
				} else {
					echo nl2br(SQ_MS_SEND_MAIL_BLACK_LIST);
				}
				$o->note(translate('messaging_service_mail_black_list_note'));
			$o->closeField();
			$o->sectionNote(translate('messaging_service_section_note'));
		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


}//end class

?>
