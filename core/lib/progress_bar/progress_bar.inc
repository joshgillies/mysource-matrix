<?php

/**
* Progress Bar
*
* //// NOTE ////
*   construct by REFERENCE, otherwise unexpected results could eventuate
*   because the _obj will be a copy of original
*
* Example
*	require_once SQ_LIB_PATH.'/progress_bar/progress_bar.inc';
*	$pb = new Progress_Bar($o->getCurrentLocation(), $this);
*	if (!$pb->loaded) {
*		$pb->setFns('_dupeInit', '_dupeInitPaint', '_dupeDone');
*		$pb->setOption('auto_close', false);
*
*		$pb->addStep('Obtain Locks', '_dupeObtainLocks');
*		$pb->addStep('Duplicate', '_dupeExec');
*		$pb->addStep('Release Locks', '_dupeReleaseLocks', '_dupeReleaseLocksInit');
*	}
*
*	$pb->run();
*
*	if ($pb->inPopUp()) {
*		$o->openRaw();
*	} else {
*		$o->setHeading('Duplicate : '.$asset->name);
*		$o->openSection('Options');
*		$o->openField('');
*	}
*
*	$pb->paint();
*
*	if ($pb->inPopUp()) {
*		$o->closeRaw();
*	} else {
*		$o->closeSection();
*		$o->commitButton();
*	}
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
* @subpackage lib
*/
class Progress_Bar
{

	/**
	* Were we loaded from the session ? If not then our owner will need to set all the information
	* @var bool
	*/
	var $loaded = false;

	/**
	* If we are testing then there are a couple of things that can get changed in the pop-up
	* Bit wise operator :
	*   1 : able to resize pop-up
	*   2 : confirm the reloading of pop-up or not
	*   4 : don't auto reload, have to be done manually
	* @var int
	*/
	var $testing = 0;

	/**
	* Some random key that we use to identify this Progress Bar in this session
	* @var string
	*/
	var $_key = '';

	/**
	* The url that is guaranteed to get us back to the coding point that we are called from
	* @var string
	*/
	var $_url = '';

	/**
	* Our owner, used for call-back fns, both global and in individual steps
	* @var object
	*/
	var $_obj = null;

	/**
	* Name of global initialisation function, if this function returns false then $this->_init_paint_fn will be called when we paint
	* @var string
	*/
	var $_init_fn = null;

	/**
	* Name of global function called when global initialisation failed, can be used to get info from user to fix any params etc, $this->_init_fn will be called again after submission
	* @var string
	*/
	var $_init_paint_fn = null;

	/**
	* Name of global function after all steps have been completed
	* @var string
	*/
	var $_done_fn = null;

	/**
	* Holds all the inforation about the individual steps
	* @var Array
	* @see addStep()
	*/
	var $_steps = Array();

	/**
	* Holds the index in _steps for the current step
	* @var Array
	* @see run()
	*/
	var $_step = 0;

	/**
	* Some global options that affect how the progress bar appears and does things
	* @var Array
	*/
	var $_options = Array(	'on_complete_url'	=> '',
							'auto_close'		=> false,
							'colour'			=> '#000099'
						);

	/**
	* Are we in the pop-up
	* @var bool
	*/
	var $_in_pop_up		= false;

	/**
	* Have we been initialised
	* @var bool
	*/
	var $_initialised	= false;

	/**
	* Have all steps been done and the done function run ?
	* @var bool
	*/
	var $_done			= false;

	/**
	* Has something has gone a-mis in the execution of the progress bar ?
	* @var bool
	*/
	var $_abort			= false;


	/**
	* Array that stores any running variables that any of the call-back functions use
	* @var Array()
	*/
	var $_running_vars	= Array();


	/**
	* Constructor
	*
	* @param  string	$url
	* @param  object	$obj
	*/
	function Progress_Bar($url, &$obj)
	{
		$this->_url     = $url;
		$this->_obj     = &$obj;

		if (empty($_REQUEST['progress_bar_key'])) {
			$this->_key = md5(uniqid(rand(),1));
		} else {
			$this->_key       = $_REQUEST['progress_bar_key'];
			$this->_in_pop_up = !empty($_REQUEST['progress_bar_in_pop_up']);
			if (!$this->_load()) {
				trigger_error('Progress Bar Load FAILED', E_USER_ERROR);
			}
		}

	}// end constructor()

	/**
	* Saves the information for this progress bar into the session
	*/
	function _save()
	{
		if (empty($_SESSION['progress_bar'])) $_SESSION['progress_bar'] = Array();

		// if we are done, remove our selves from the session
		if ($this->_done || $this->_abort) {
			unset($_SESSION['progress_bar'][$this->_key]);
		// otherwise save as usual
		} else {
			$vars = get_object_vars($this);
			unset($vars['testing']);
			unset($vars['_loaded']);
			unset($vars['_url']);
			unset($vars['_obj']);
			unset($vars['_in_pop_up']);
			$_SESSION['progress_bar'][$this->_key] = $vars;
		}// end if

	}// end _save()


	/**
	* Loads the information for this progress bar from the session
	*/
	function _load()
	{
		if (empty($_SESSION['progress_bar'][$this->_key])) return false;
		foreach($_SESSION['progress_bar'][$this->_key] as $k => $v) {
			$this->$k = $v;
		}
		$this->loaded = true;
		return true;

	}// end _load()


	/**
	* Sets the global call-back fns
	* Can only be called if $this->loaded is false
	*
	* @param string	$init_fn
	* @param string	$init_paint_fn
	* @param string	$done_fn
	*/
	function setFns($init_fn=null, $init_paint_fn=null, $done_fn=null)
	{
		if ($this->loaded) return;

		$this->_init_fn = $init_fn;
		$this->_initialised = is_null($init_fn); // if there is no init fn, we are initalised
		// can only have a init paint fn if we have an init fn
		if (!is_null($init_fn)) {
			$this->_init_paint_fn = $init_paint_fn;
		}
		$this->_done_fn = $done_fn;

	}// end setFns()


	/**
	* Adds a step into the progress bar
	* Can only be called if $this->loaded is false
	*
	* @param string	$title		title/label given to this step
	* @param string	$exec_fn	call-back function to execute for as many times as is necessary until it set's the percentage to 100
	* @param string	$init_fn	call-back function to do any initialisation stuff for this step
	* @param string	$done_fn	call-back function executed after percentage for this step hit's 100
	*/
	function addStep($title, $exec_fn, $init_fn=null, $done_fn=null)
	{
		if ($this->loaded) return;

		$this->_steps[] = Array('title'			=> $title,
								'exec_fn'		=> $exec_fn,
								'init_fn'		=> $init_fn,
								'initialised'   => is_null($init_fn), // if there is no init fn, we are initalised
								'done_fn'		=> $done_fn,
								'done'          => false,
								'status_msg'	=> '',
								'percent'		=> 0.0);

	}// end addStep()


	/**
	* Set's one of the global options for the progress bar
	* Can only be called if $this->loaded is false
	*
	* @param string	$name		name of the option
	* @param string	$value		new value to assign it
	*/
	function setOption($name, $value)
	{
		if ($this->loaded) return;

		if (!isset($this->_options[$name])) {
			trigger_error('Progess Bar Option "'.$name.'" not known', E_USER_WARNING);
			return;
		}
		$this->_options[$name] = $value;

	}// end setOption()


	/**
	* Returns whether we are in the pop-up or not
	* Useful for differing what you wrap around the paint fn
	*
	* @return bool
	*/
	function inPopUp()
	{
		return $this->_in_pop_up;
	}// end inPopUp()


	/**
	* Set's the status message for the current step
	*
	* @param string	$status_msg
	*/
	function setStatusMsg($status_msg)
	{
		if (!isset($this->_steps[$this->_step])) return;
		$this->_steps[$this->_step]['status_msg'] = $status_msg;
	}// end setStatusMsg()


	/**
	* Set's the percentage for the current step
	*
	* @param float	$percent	a number between 0 and 1
	*/
	function setPercent($percent)
	{
		if (!isset($this->_steps[$this->_step])) return;
		$this->_steps[$this->_step]['percent'] = min(max(0.0, (float) $percent), 1.0) * 100.0;
	}// end setPercent()


	/**
	* Resets the vars that the call-back fns use
	*
	* @param Array	$vars
	*/
	function setRunningVars($vars)
	{
		$this->_running_vars = $vars;
	}// end setRunningVars()


	/**
	* Returns the vars that the call-back fns use
	*
	* @return Array
	*/
	function getRunningVars()
	{
		return $this->_running_vars;
	}// end getRunningVars()


	/**
	* Returns the vars that the call-back fns use
	*
	* @return Array
	*/
	function abort()
	{
		if (!$this->inPopUp()) {
			trigger_error('You cannot call abort when we are not in the pop up', E_USER_ERROR);
		}
		return $this->_abort = true;
	}// end abort()


	/**
	* Runs the difference call-back function depending on what the current status of the progress bar is
	*/
	function run()
	{
		// if we have aborted get out of here
		if ($this->_abort) return;

		// have we initialised globally ?
		if (!$this->_initialised) {
			$fn = $this->_init_fn;
			$this->_initialised = $this->_obj->$fn($this);

		// if we haven't done the last step
		// we have we got more todo
		} elseif ($this->_step < count($this->_steps)) {

			$step = &$this->_steps[$this->_step];

			// have we initialised this step ?
			if (!$step['initialised']) {
				$fn = $step['init_fn'];
				$this->_obj->$fn($this);
				// always initialised in these because we don't have any paint fn
				$step['initialised'] = true;

			// have we got more todo in this step ?
			} elseif ($step['percent'] < 100.0) {
				$fn = $step['exec_fn'];
				$this->_obj->$fn($this);

				// if we have finished the looping and there is no done fn,
				// then we have done this step (do this so we don't need to waste a browser refresh)
				if ($step['percent'] == 100.0 && is_null($step['done_fn'])) {
					$step['done'] = true;
					$this->_step++;
				}

			// we must be finished this step, run it's done command
			} elseif (!is_null($step['done_fn'])) {
				$fn = $step['done_fn'];
				$this->_obj->$fn($this);

				// mark as done and go to next step
				$step['done'] = true;
				$this->_step++;

			}// end if

		// we must be finished, run global done command
		} elseif (!$this->_done) {
			if (!is_null($this->_done_fn)) {
				$fn = $this->_done_fn;
				$this->_obj->$fn($this);
			}
			$this->_done = true;

		}// end if

		$this->_save();

	}// end run()


	/**
	* Returns a url to use for the pop-up
	*
	* @return string
	*/
	function popUpUrl()
	{
		return $this->_url
				.'&progress_bar_key='.$this->_key
				.'&progress_bar_in_pop_up=1'
				.'&progress_bar_time='.rawurlencode(microtime())
				.'.#progress_bar_'.(($this->_step < count($this->_steps)) ? $this->_step : 'done');

	}// end popUpUrl()


	/**
	* Paint's either the progress bar, it's initiating code or the _init_paint_fn
	* depending on what's happening
	*/
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

			$scrollbars = ($this->testing & 1);

			// size of progress bars is about 75 pixels
			$window_height = (75 * count($this->_steps));
			// size of close button is about 30 pixels
			if ($this->_options['auto_close']) $window_height += 30;

			// just to be on the safe side
			if ($window_height > 400) {
				$window_height = 400;
				$scrollbars = true;
			}

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
						PROGRESS_BAR.popup = window.open(PROGRESS_BAR.url, PROGRESS_BAR.key, 'height=<?php echo $window_height; ?>,width=400,toolbar=no,location=no,menubar=no,resizable=<?php echo ($scrollbars) ? 'yes' : 'no'; ?>,nominimize,nomaximize,norestore,scrollbars=yes');
						PROGRESS_BAR.popup.focus();

						PROGRESS_BAR.form.elements['progress_bar_btn'].value = 'Running...';

					}// end progress_bar_start()

					function progress_bar_update(step, title, status_msg, percent) {
						if (!PROGRESS_BAR.started) {
							alert('Progress Action has not been started, therefore it cannot be updated');
							return false;
						}

						var msg =
				<?php
					if (count($this->_steps) > 1) {
				?>

						"Step " + step + " of <?php echo count($this->_steps); ?>\n" +
				<?php
					}
				?>

							title + "\n"
							+ status_msg + "\n"
							+ percent + "%";

						PROGRESS_BAR.form.elements['progress_bar_msgbox'].value = msg;

					}// end progress_bar_update()

					function progress_bar_end() {
						if (!PROGRESS_BAR.started) {
							alert('Progress Action has not been started, therefore it cannot end');
							return false;
						}

						PROGRESS_BAR.done = true;
						PROGRESS_BAR.form.elements['progress_bar_btn'].value = 'Done';

			<?php
				if ($this->_options['on_complete_url'] != '') {
				?>
						self.location = '<?php echo addslashes($this->_options['on_complete_url']); ?>';
				<?php
				}
			?>
					}// end progress_bar_end()

				 // -->
			</script>
			<?php

			$msg  = " \n \n0.0%";
			$rows = 3;
			if (count($this->_steps) > 1) {
				$msg = 'Step 0 of '.count($this->_steps)."\n".$msg;
				$rows++;
			}
			text_area('progress_bar_msgbox', $msg, 30, $rows, 'style="border-style: none; border-width: 0px;" onFocus="javascript: this.temp_value = this.value;" onBlur="javascript: this.value = this.temp_value;"');
			echo '<br/>';
			normal_button('progress_bar_btn', 'Start...', 'progress_bar_start(this.form);');

		// else we are in the popup
		} else {

			// if we have aborted get out of here
			if($this->_abort) {
				trigger_error('Progress Bar Aborted', E_USER_WARNING);
				return;
			}

			?>
			<script language="Javascript" type="text/javascript">
				<!--
			<?php
			if ($this->_step < count($this->_steps)) {
			?>

					if (window.opener && window.opener.progress_bar_update) {
						window.opener.progress_bar_update('<?php echo ($this->_step + 1); ?>',
														'<?php echo addslashes($this->_steps[$this->_step]['title']); ?>',
														'<?php echo addslashes($this->_steps[$this->_step]['status_msg']); ?>',
														'<?php echo round($this->_steps[$this->_step]['percent']); ?>'
														);
					}
			<?php
			}// end if
			// if we haven't run the done fn, loop again
			if(!$this->_done) {
			?>
					function progress_bar_onload() {

						PROGRESS_BAR_other_onload();
					<?php if ($this->testing & 2) { ?>
						if (!confirm("Go ?")) return;
					<?php } // end if ?>
					<?php if (!($this->testing & 4)) { ?>
						// pause for 1/100th of a second to let the server have a break
						setTimeout("self.location = '<?php echo addslashes($this->popUpUrl()); ?>';", 10);
					<?php } // end if ?>


					}// end progress_bar_onunload()

					var PROGRESS_BAR_other_onload = (window.onload) ? window.onload :  new Function;
					window.onload = progress_bar_onload;
			<?php
			}// end if not done
			?>
				 // -->
			</script>
		<?php
			for($i = 0, $num_steps = count($this->_steps); $i < $num_steps; $i++) {
				$step = $this->_steps[$i];
		?>
			<table width="100%">
				<tr>
					<td colspan="2" style="font-size: 11px; color: #000000; font-family: verdana; font-weight: bold;">
						<a name="progress_bar_<?php echo $i; ?>"></a><?php echo $step['title']; ?>
					</td>
				</tr>
				<tr>
					<td style="font-size: 10px; color: #000000; font-family: verdana; font-weight: bold;">
						<?php echo $step['status_msg'];?>
					</td>
					<td style="font-size: 10px; color: #000000; font-family: verdana; font-weight: bold; text-align: right;">
						<?php echo round($step['percent']); ?>%
					</td>
				</tr>
				<tr>
					<td colspan="2" style="text-align: left; background-color: #ffffff;">
				<?php
					if ($step['percent']) {
					?>
						<table width="<?php echo round($step['percent']); ?>%" cellspacing="0" cellpadding="0" border="0">
							<tr>
								<td style="background-color: <?php echo $this->_options['colour'];?>;">&nbsp;</td>
							</tr>
						</table>
					<?php
					} else {
					?>
						&nbsp;
					<?php
					}// end if
				?>
					</td>
				</tr>
			</table>
		<?php
			}// end for

			// if we have run the done fn, then that's all folks
			if($this->_done) {
			?>

			<table width="100%">
				<tr>
					<td colspan="2" style="font-size: 10px; color: #000000; font-family: verdana; font-weight: bold; text-align: right;">
						<a name="progress_bar_done"></a>
						<script language="Javascript" type="text/javascript">
							<!--
								function progress_bar_done() {
									if (window.opener && window.opener.progress_bar_update) {
										window.opener.progress_bar_update('<?php echo count($this->_steps); ?>',
																		'',
																		'',
																		'100'
																		);
									}
									if (window.opener && window.opener.progress_bar_end) {
										window.opener.progress_bar_end();
									}

							<?php
								if ($this->_options['auto_close']) {
								?>

									window.close();
								<?php
								}
							?>

								}// end progress_bar_done()

								window.onload = function(){ setTimeout("progress_bar_done()", 10) };

							 // -->
						</script>
						<?php
						if (!$this->_options['auto_close']) {
							normal_button('progress_bar_btn', 'Close', 'window.close();');
						}
						?>
					</td>
				</tr>
			</table>
			<?php
			}// end if

		}// end if

		$this->_save();

	}// end paint()

}// end class

?>