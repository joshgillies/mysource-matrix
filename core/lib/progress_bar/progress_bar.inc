<?php


/**
* Construct BY REFERENCE
*/


class Progress_Bar
{

	var $_key = '';

	var $_url = '';

	var $_obj = null;

	var $_init_fn = '';
	var $_init_paint_fn = '';
	var $_exec_fn = '';
	var $_done_fn = '';

	var $_colour = '';


	var $_in_pop_up		= false;
	var $_initialised	= false;
	var $_done			= false;

	var $_title			= '';
	var $_status_msg	= '';
	var $_percent		= 0.0;

	var $_running_vars = Array();

	function Progress_Bar($url, &$obj, $init_fn, $init_paint_fn, $exec_fn, $done_fn, $colour='#000099')
	{
		$this->_url     = $url;

		$this->_obj     = &$obj;
		$this->_init_fn = $init_fn;
		$this->_init_paint_fn = $init_paint_fn;
		$this->_exec_fn = $exec_fn;
		$this->_done_fn = $done_fn;
		$this->_colour  = $colour;

		if (empty($_REQUEST['progress_bar_key'])) {
			$this->_key = md5(uniqid(rand(),1));
		} else {
			$this->_key       = $_REQUEST['progress_bar_key'];
			$this->_in_pop_up = !empty($_REQUEST['progress_bar_in_pop_up']);
			if (!$this->_load()) {
				trigger_error('Progress Bar Load FAILED', E_USER_ERROR);
			}
		}

	}// end constructor

	function _save()
	{
		if (empty($_SESSION['progress_bar'])) $_SESSION['progress_bar'] = Array();

		$vars = get_object_vars($this);
		unset($vars['_url']);
		unset($vars['_obj']);
		unset($vars['_init_fn']);
		unset($vars['_init_paint_fn']);
		unset($vars['_exec_fn']);
		unset($vars['_done_fn']);
		unset($vars['_colour']);
		unset($vars['_in_pop_up']);

		$_SESSION['progress_bar'][$this->_key] = $vars;
	}

	function _load()
	{
		if (empty($_SESSION['progress_bar'][$this->_key])) return false;
		foreach($_SESSION['progress_bar'][$this->_key] as $k => $v) {
			$this->$k = $v;
		}
		return true;
	}

	function inPopUp()
	{
		return $this->_in_pop_up;
	}

	function setTitle($title)
	{
		$this->_title = $title;
	}

	function setStatusMsg($status_msg)
	{
		$this->_status_msg = $status_msg;
	}

	function setPercent($percent)
	{
		$this->_percent = min(max(0.0, (float) $percent), 1.0) * 100.0;
	}

	function setRunningVars($vars)
	{
		$this->_running_vars = $vars;
	}

	function getRunningVars()
	{
		return $this->_running_vars;
	}

	function run()
	{
		// have we initialised
		if (!$this->_initialised) {
			$fn = $this->_init_fn;
			$this->_initialised = $this->_obj->$fn($this);

		// have we got more todo ?
		} elseif ($this->_percent < 100) {
			$fn = $this->_exec_fn;
			$this->_obj->$fn($this);

		// we must be finished
		} else {
			$fn = $this->_done_fn;
			$this->_obj->$fn($this);
			$this->_done = true;

		}// end if

		$this->_save();

	}// end run()

	function popUpUrl()
	{
		return $this->_url
				.'&progress_bar_key='.$this->_key
				.'&progress_bar_in_pop_up=1'
				.'&progress_bar_time='.rawurlencode(microtime());

	}

	function paint()
	{
		static $run = false;
		if ($run) {
			trigger_error('Paint has already been called for a Progress Bar on this page, you can only have one progress bar per script execution', E_USER_WARNING);
			return;
		}
		$run = true;

		if (!$this->_initialised) {

			hidden_field('progress_bar_key', $this->_key);
			$fn = $this->_init_paint_fn;
			$this->_obj->$fn($this);

		} else if (!$this->inPopUp()) {
			pre_echo('CHANGE FROM USING HTML_FORM');

			?>
			<script language="Javascript" type="text/javascript">
				<!--
					var PROGRESS_BAR = {
							key: '<?php echo addslashes($this->_key); ?>',
							url: '<?php echo addslashes($this->popUpUrl()); ?>',
							started: false,
							done: false,
							pop_up: null,
							form: null,
							onunload_fn: null
						};
					function progress_bar_start(form) {
						if (PROGRESS_BAR.started) {
							if (PROGRESS_BAR.done) {
								alert('You have already done this action');
							} else {
								alert('You have already started this action');
							}
							return false;
						}

						PROGRESS_BAR.form = form;

						PROGRESS_BAR.started = true;
						PROGRESS_BAR.popup = window.open(PROGRESS_BAR.url, PROGRESS_BAR.key, 'toolbar=no,location=no,menubar=no,resizable=no,width=400,height=400,nominimize,nomaximize,norestore,scrollbars=no');
						PROGRESS_BAR.popup.focus();

						PROGRESS_BAR.form.elements['progress_bar_btn'].value = 'Running...';

					}// end progress_bar_start()

					function progress_bar_update(title, status_msg, percent) {
						if (!PROGRESS_BAR.started) {
							alert('Progress Action has not been started, therefore it cannot be updated');
							return false;
						}

						PROGRESS_BAR.form.elements['progress_bar_title'].value = title;
						PROGRESS_BAR.form.elements['progress_bar_status_msg'].value = status_msg;
						PROGRESS_BAR.form.elements['progress_bar_percent'].value = percent + '%';

					}// end progress_bar_update()

					function progress_bar_end() {
						if (!PROGRESS_BAR.started) {
							alert('Progress Action has not been started, therefore it cannot end');
							return false;
						}

						PROGRESS_BAR.done = true;
						PROGRESS_BAR.form.elements['progress_bar_btn'].value = 'Done';

					}// end progress_bar_end()

				 // -->
			</script>
			<?php

			$extras = 'style="border-style: none; border-width: 0px;" onFocus="javascript: this.tmp = this.value;" onBlur="javascript: this.value = this.tmp;"';

			text_box('progress_bar_title',   '', 30, 0, $extras);
			echo '<br/>';
			text_box('progress_bar_status_msg',  '', 30, 0, $extras);
			echo '<br/>';
			text_box('progress_bar_percent', '0.0%', 10, 0, $extras);
			echo '<br/>';
			normal_button('progress_bar_btn', 'Start...', 'progress_bar_start(this.form);');

		} else {

			?>
			<script language="Javascript" type="text/javascript">
				<!--
					if (window.opener && window.opener.progress_bar_update) { 
						window.opener.progress_bar_update('<?php echo addslashes($this->_title); ?>', 
														'<?php echo addslashes($this->_status_msg); ?>', 
														'<?php echo addslashes($this->_percent); ?>'
														); 
					}

			<?php
			// if we haven't run the done fn, loop again
			if(!$this->_done) {
			?>
 					function progress_bar_onload() {

						PROGRESS_BAR_other_onload();
						//if(!prompt('load ? ', '<?php echo addslashes($this->popUpUrl()); ?>')) return;
						self.location = '<?php echo addslashes($this->popUpUrl()); ?>';

					}// end progress_bar_onunload()

					var PROGRESS_BAR_other_onload = (window.onload) ? window.onload :  new Function;
					window.onload = progress_bar_onload;
			<?php
			}// end if not done
			?>
 				 // -->
			</script>
			<table width="100%">
				<tr>
					<td colspan="2" style="font-size: 11px; color: #000000; font-family: verdana; font-weight: bold;">
						<?php echo round($this->_percent); ?>% completed
					</td>
				</tr>
				<tr>
					<td width="<?php echo round($this->_percent); ?>%" style="background-color: <?php echo $this->_colour;?>;">&nbsp;</td>
					<td style="background-color: #ffffff;">&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2" style="font-size: 10px; color: #000000; font-family: verdana; font-weight: bold; text-align: right;">
						<?php echo $this->_status_msg;?>
					</td>
				</tr>
			<?php

			// if we have finished, and we have run the done fn, then that's all folks
			if($this->_percent == 100.0 && $this->_done) {
				?>

				<tr>
					<td colspan="2" style="font-size: 10px; color: #000000; font-family: verdana; font-weight: bold; text-align: right;">
						<script language="Javascript" type="text/javascript">
							<!--
								function progress_bar_done() {
									if (window.opener && window.opener.progress_bar_end) { 
										window.opener.progress_bar_end();
									}

									window.close();

								}// end progress_bar_onunload()

							 // -->
						</script>
						<?php 
							normal_button('progress_bar_btn', 'Close', 'progress_bar_done();');
						?>
					</td>
				</tr>
				<?php
			}// end if
			?>

			</table>
			<?php

		}// end if

		$this->_save();

	}// end paint()

}// end class

?>