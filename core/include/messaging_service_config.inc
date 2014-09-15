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
* $Id: messaging_service_config.inc,v 1.33 2013/09/17 00:07:51 cupreti Exp $
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
* @version $Revision: 1.33 $
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
						'SQ_MS_SEND_MAIL_WHITE_LIST'	=> Array('editable' => 1, 'default' => "asset.locking.forced\nconfig.*\nhipo.*\ninbox.*\nasset.status.notify\nasset.workflow.invitation\nasset.workflow.announce\nasset.workflow.invitation.*\nasset.workflow.announce.*\nasset.workflow.stale\nasset.workflow.bundle.*\ncron.deadlock"),
						'SQ_MS_SEND_MAIL_BLACK_LIST'	=> Array('editable' => 1, 'default' => ''),
						'SQ_MS_EMAIL_FOOTER'				=> Array('editable' => 1, 'default' => ''),
					   );


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();
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
		$class = get_class_lower($this);

		require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';

		$option_list = new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);
		$option_list->setEditParam('width', 60);

		$o->openSection(translate('messaging_service_configuration'));
			$o->openField(translate('messaging_service_file_white_list'));
				if ($write_access) {
					$option_list->value = SQ_MS_LOG_TO_FILE_WHITE_LIST;
					$option_list->paint('SQ_MS_LOG_TO_FILE_WHITE_LIST');
				} else {
					echo nl2br(SQ_MS_LOG_TO_FILE_WHITE_LIST);
				}
				$o->note(translate('messaging_service_file_white_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_file_black_list'));
				if ($write_access) {
					$option_list->value = SQ_MS_LOG_TO_FILE_BLACK_LIST;
					$option_list->paint('SQ_MS_LOG_TO_FILE_BLACK_LIST');
				} else {
					echo nl2br(SQ_MS_LOG_TO_FILE_BLACK_LIST);
				}
				$o->note(translate('messaging_service_file_black_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_db_white_list'));
				if ($write_access) {
					$option_list->value = SQ_MS_LOG_TO_DB_WHITE_LIST;
					$option_list->paint('SQ_MS_LOG_TO_DB_WHITE_LIST');
				} else {
					echo nl2br(SQ_MS_LOG_TO_DB_WHITE_LIST);
				}
				$o->note(translate('messaging_service_db_white_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_db_black_list'));
				if ($write_access) {
					$option_list->value = SQ_MS_LOG_TO_DB_BLACK_LIST;
					$option_list->paint('SQ_MS_LOG_TO_DB_BLACK_LIST');
				} else {
					echo nl2br(SQ_MS_LOG_TO_DB_BLACK_LIST);
				}
				$o->note(translate('messaging_service_db_black_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_mail_white_list'));
				if ($write_access) {
					$option_list->value = SQ_MS_SEND_MAIL_WHITE_LIST;
					$option_list->paint('SQ_MS_SEND_MAIL_WHITE_LIST');
				} else {
					echo nl2br(SQ_MS_SEND_MAIL_WHITE_LIST);
				}
				$o->note(translate('messaging_service_mail_white_list_note'));
			$o->closeField();
			$o->openField(translate('messaging_service_mail_black_list'));
				if ($write_access) {
					$option_list->value = SQ_MS_SEND_MAIL_BLACK_LIST;
					$option_list->paint('SQ_MS_SEND_MAIL_BLACK_LIST');
				} else {
					echo nl2br(SQ_MS_SEND_MAIL_BLACK_LIST);
				}
				$o->note(translate('messaging_service_mail_black_list_note'));
			$o->closeField();
		$o->closeSection();

		$o->openSection(translate('messaging_service_section_email_format'));
			$o->openField(translate('messaging_service_email_footer'));
				if ($write_access) {
					text_area($class.'[SQ_MS_EMAIL_FOOTER]', SQ_MS_EMAIL_FOOTER, 80, 5);
				} else {
					echo nl2br(SQ_MS_EMAIL_FOOTER);
				}
				$o->note(translate('messaging_service_email_footer_note'));
			$o->closeField();
		$o->closeSection();

		if ($write_access) $o->commitButton('', TRUE);

	}//end paintBackend()


	/**
	* Saves the config settings that were submitted
	*
	* @param object Backend_Outputter	&$o			reference to the backend outputter
	*
	* @return boolean
	* @access public
	* @see paintBackend()
	*/
	function processBackend(&$o)
	{
		$write_access = $this->writeAccess();
		if (!$write_access) return parent::processBackend($o);

		$class = get_class_lower($this);

		require_once SQ_ATTRIBUTES_PATH.'/option_list/option_list.inc';
		$option_list = new Asset_Attribute_Option_List(0, NULL, SQ_OPTION_LIST_DELIMITER_UNIX);

		// We assume namining of these constants, and that they always have white/black lists.
		$message_types = Array(
							'LOG_TO_FILE',
							'LOG_TO_DB',
							'SEND_MAIL',
						 );

		foreach ($message_types as $message_type) {
			// Names for both white and black list for this message type.
			$w = 'SQ_MS_'.$message_type.'_WHITE_LIST';
			$b = 'SQ_MS_'.$message_type.'_BLACK_LIST';

			// Process attribute. Value is string seperated by newlines (\n).
			$option_list->process($w);
			$_POST[$class][$w] = $option_list->value;

			$option_list->process($b);
			$_POST[$class][$b] = $option_list->value;
		}
		return parent::processBackend($o);

	}//end processBackend()


}//end class

?>
