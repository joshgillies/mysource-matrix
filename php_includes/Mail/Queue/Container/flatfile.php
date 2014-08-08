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
* $Id: flatfile.php,v 1.7 2012/08/30 01:11:22 ewang Exp $
*
*/

require_once 'Mail/Queue/Container.php';


/**
 * Mail_Queue_Container_flatfile - Storage driver for fetching mail queue data
 * from flatfile storage
 *
 * @author   Nathan de Vries <ndvries@squiz.net>
 * @package  Mail_Queue
 * @version  $Revision: 1.7 $
 * @access   public
 */
class Mail_Queue_Container_flatfile extends Mail_Queue_Container
{

	/**
	 * Directory where the queue is being stored
	 * @var string
	 */
	var $dir;


	/**
	 * Contructor
	 *
	 * Mail_Queue_Container_flatfile:: Mail_Queue_Container_flatfile()
	 *
	 * @param mixed $options    An associative array of option names and
	 *                          their values. For now, just 'dir'.
	 *
	 * @access public
	 */
	function Mail_Queue_Container_flatfile($options)
	{
		if (!is_array($options) || !isset($options['dir'])) {
			return new Mail_Queue_Error(MAILQUEUE_ERROR_NO_OPTIONS,
				$this->pearErrorMode, E_USER_ERROR, __FILE__, __LINE__,
				'No queue directory specified!');
		}
		$this->dir = $options['dir'];
		$this->setOption();
	}


	/**
	 * Preload mail to queue.
	 *
	 * @return mixed  True on success else Mail_Queue_Error object.
	 * @access private
	 */
	function _preload()
	{
		$this->_last_item = 0;
		$this->queue_data = array(); //reset buffer

		$queue_files = $this->list_files($this->dir);
		foreach ($queue_files as $index => $messageID) {
			if ($this->limit && ($index + 1 > $this->limit)) break;

			$mail_array = $this->loadFromQueue($messageID);
			$mail_body = new Mail_Queue_Body(
								$mail_array['id'],
								$mail_array['create_time'],
								$mail_array['time_to_send'],
								$mail_array['sent_time'],
								$mail_array['id_user'],
								$mail_array['ip'],
								$mail_array['sender'],
								$mail_array['recipient'],
								unserialize($mail_array['headers']),
								unserialize($mail_array['body']),
								$mail_array['delete_after_send'],
								$mail_array['try_sent']
							  );
			if (is_a($mail_body, 'Mail_Queue_Body')) {
				$this->queue_data[$this->_last_item] = $mail_body;
				$this->_last_item++;
			}
		}

		return true;

	}//end _preload()


	/**
	 * List all the filename of files in a particular directory
	 *
	 * @param string	The directory to look for files
	 *
	 * @return array	A numerically indexed array of filenames
	 * @access private
	 */
	function list_files($dir)
	{
		if (!is_dir($dir)) return Array();

		$files = Array();
		if ($handle = opendir($dir)) {
			while (($file = readdir($handle)) !== false) {
				if ($file == '.' || $file == '..') {
					continue;
				} else  if (is_file($dir.'/'.$file)) {
					$files[] = $file;
				}
			}
			closedir($handle);
		}
		return $files;

	}//end list_files()


	/**
	 * Put new mail in queue and save in database.
	 *
	 * Mail_Queue_Container::put()
	 *
	 * @param string $time_to_send  When mail have to be send
	 * @param integer $id_user  Sender id
	 * @param string $ip  Sender ip
	 * @param string $from  Sender e-mail
	 * @param string $to  Reciepient e-mail
	 * @param string $hdrs  Mail headers (in RFC)
	 * @param string $body  Mail body (in RFC)
	 * @param bool $delete_after_send  Delete or not mail from filesystem after send
	 *
	 * @return mixed  ID of the record where this mail has been put
	 *                or Mail_Queue_Error on error
	 * @access public
	 **/
	function put($time_to_send, $id_user, $ip, $sender,
				$recipient, $headers, $body, $delete_after_send=true)
	{
		// accommodate changes made to PEAR Mail_Queue v 1.2.1
		$to = $recipient;
		if (method_exists($this, '_isSerialized')) {
			if ($this->_isSerialized($recipient)) {
				$to = unserialize($recipient);
			}
		}

		// uses user_id as the custom id of mail queue files
		$header_array = unserialize($headers);
		if (isset($header_array['custom_id'])) {
			$custom_id = substr($header_array['custom_id'], 0, 113);
			$id = md5(uniqid($custom_id));
			unset($header_array['custom_id']);
			$headers = serialize($header_array);
		} else {
			$id = md5(uniqid(''));
		}

		$array = Array(
					'id'				=> $id,
					'time_to_send'		=> $time_to_send,
					'id_user'			=> $id_user,
					'ip'				=> $ip,
					'sender'			=> $sender,
					'recipient'			=> $to,
					'headers'			=> $headers,
					'body'				=> $body,
					'delete_after_send'	=> $delete_after_send,
					'try_sent'			=> 0,
					'create_time'		=> date("Y-m-d G:i:s", time()),
					'sent_time'			=> 0,
				 );

		$this->saveToQueue($id, $array);

		return $id;
	}

	/**
	 * Load the mail data from the file system queue
	 *
	 * @param int	MailID of the data we are trying to load
	 *
	 * @return array	The result of including the file if it exists, or an empty array
	 * @access private
	 */
	function loadFromQueue($id) {
		if (is_file($this->dir.'/'.$id)) {
			require $this->dir.'/'.$id;
		}
		return isset($mail_entry) ? $mail_entry : Array();
	}

	/**
	 * Save the mail data to a single file so it can be included later.
	 *
	 * @param int	MailID of the data we are trying to load
	 * @param array	Array of data to save
	 *
	 * @return int	The result of fwrite()ing the file
	 * @access private
	 */
	function saveToQueue($id, $contents) {
		$output = '<?php $mail_entry = '.var_export($contents, true).'; ?>';
		$fp = fopen($this->dir.'/'.$id, 'w');
		$return_value = fwrite($fp, $output);
		fclose($fp);
		return $return_value;
	}


	/**
	 * Check how many times mail was sent.
	 *
	 * @param object   Mail_Queue_Body
	 * @return mixed  Integer or Mail_Queue_Error class if error.
	 * @access public
	 */
	function countSend($mail)
	{
		if (!is_object($mail) || !is_a($mail, 'mail_queue_body')) {
			return new Mail_Queue_Error('Expected: Mail_Queue_Body class',
				__FILE__, __LINE__);
		}
		$count = $mail->_try();

		$mail_array = $this->loadFromQueue($mail->getId());
		if (!empty($mail_array)) {
			$mail_array['try_sent'] = $count;
			$this->saveToQueue($mail->getId(), $mail_array);
		}

		return $count;
	}


	/**
	 * Set mail as already sent.
	 *
	 * @param object Mail_Queue_Body object
	 * @return bool
	 * @access public
	 */
	function setAsSent($mail)
	{
		if (!is_object($mail) || !is_a($mail, 'mail_queue_body')) {
			return new Mail_Queue_Error(MAILQUEUE_ERROR_UNEXPECTED,
				$this->pearErrorMode, E_USER_ERROR, __FILE__, __LINE__,
				'Expected: Mail_Queue_Body class');
		}

		$mail_array = $this->loadFromQueue($mail->getId());
		if (!empty($mail_array)) {
			$mail_array['sent_time'] = time();
			$this->saveToQueue($mail->getId(), $mail_array);
		}

		return true;
	}


	/**
	 * Remove from queue mail with $id identifier.
	 *
	 * @param integer $id  Mail ID
	 * @return bool  True on success else Mail_Queue_Error class
	 *
	 * @access public
	 */
	function deleteMail($id)
	{
		// move files to .queue/.sent folder instead of deleting them
		$success = rename($this->dir.'/'.$id, $this->dir.'/sent/'.$id);

		if ($success === false) {
			return new Mail_Queue_Error(MAILQUEUE_ERROR_QUERY_FAILED,
				$this->pearErrorMode, E_USER_ERROR, __FILE__, __LINE__,
				'Unable to delete mail from queue (MailID #'.$id.')');
		}

		return true;
	}

}
?>
