import glob
import logging
import os
import urllib.request
import subprocess
import csv
import pandas as pd
import os

from telegram.ext import Updater, CommandHandler, MessageHandler, Filters


# Enable logging
logging.basicConfig()
logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
                    level=logging.INFO)

logger = logging.getLogger(__name__)


# Define a few command handlers. These usually take the two arguments update and
# context. Error handlers also receive the raised TelegramError object in error.
def start(update, context):
    """Send a message when the command /start is issued."""
    update.message.reply_text('Hi!')


def help(update, context):
    """Send a message when the command /help is issued."""
    update.message.reply_text('Help!')

def echo(update, context):
    """Echo the user message."""
    print("Document received")
    chat_id = update.message.chat_id
    file_id = update.message['document']['file_id']
    file_name = update.message['document']['file_name']
    print(file_name)
    file = context.bot.getFile(file_id)
    file_path = file.file_path
    urllib.request.urlretrieve(file_path, file_name)
    #update.message.reply_text(file_path)
    update.message.reply_text("Thanks")

def error(update, context):
    global is_testing
    global completed
    is_testing = False
    """Log Errors caused by Updates."""
    update.message.reply_text('Update "%s" caused error "%s"', update, context.error)
    logger.warning('Update "%s" caused error "%s"', update, context.error)

is_testing = False
completed = 0
def run_test(update, context):
    global is_testing
    global completed
    if is_testing:
        update.message.reply_text('The test is already running!')
        return
    is_testing = True
    update.message.reply_text('Started testing!')
    paths = ["../data/berlin52.tsp", "../data/eil51.tsp", "../data/att48.tsp", "../data/st70.tsp", "../data/pr76.tsp"]
    for name in glob.glob('../data/compact_models/*/*'):
        paths.append(name)
    methods = ["MTZ", "MTZL", "MTZI", "MTZLI", "MTZ_IND", "GG"]
    time_limit = "3600"
    total_runs = len(methods) * len(paths)
    csv_filename="compact.csv"

    #if file csv do not exist, create it.
    if not os.path.exists('../measures/'+csv_filename):
        df=pd.DataFrame(index=paths,columns=methods)
        df.to_csv('../measures/'+csv_filename,index=True)
    else:
        df=pd.read_csv('../measures/'+csv_filename,index_col=0)

    num_runs = 0
    chat_id = update.message.chat_id
    for tsp in paths:
        for m in methods:
            num_runs += 1
            completed = num_runs / total_runs * 100
            row = df.index.get_loc(tsp)
            if not pd.isnull(df[m].values[row]):
                continue
            str_exec = "../build/tsp -f {path} -verbose -1 -method {method} -t {time_lim} --perfprof"
            str_exec = str_exec.format(path=tsp, method=m, time_lim=time_limit)
            print("Testing on " +m+ " on instance " +tsp)
            output = subprocess.check_output(str_exec, shell=True)
            #with open('performances.csv', mode='w') as perf_file:
            output=float(output.decode("utf-8"))
            update.message.reply_text('Completed ' + tsp + " with method " + m + " in " +str(output) + " COMPLETED: " + str(completed))

            print("\t"+m+": "+str(output))
            df.loc[tsp,m]=output
            df.to_csv('../measures/'+csv_filename,index=True,mode='w+' )
    csv_file = open("../measures/"+csv_filename, "rb")
    context.bot.sendDocument(chat_id=chat_id, document=csv_file)
    is_testing = False


def get_completion(update, context):
    update.message.reply_text(str(completed) + "%")


def main():
    """Start the bot."""
    # Create the Updater and pass it your bot's token.
    # Make sure to set use_context=True to use the new context based callbacks
    # Post version 12 this will no longer be necessary
    TOKEN = "1760909698:AAFjZmRtEzv2IUj6AwhPRRNOKfTQRObjmcw"
    updater = Updater(TOKEN)

    # Get the dispatcher to register handlers
    dp = updater.dispatcher

    # on different commands - answer in Telegram
    dp.add_handler(CommandHandler("start", start))
    dp.add_handler(CommandHandler("help", help))
    dp.add_handler(CommandHandler("runtest", run_test))
    dp.add_handler(CommandHandler("completion", get_completion))
    # on noncommand i.e message - echo the message on Telegram
    dp.add_handler(MessageHandler(Filters.document, echo))

    # log all errors
    dp.add_error_handler(error)

    # Start the Bot
    updater.start_polling()

    # Run the bot until you press Ctrl-C or the process receives SIGINT,
    # SIGTERM or SIGABRT. This should be used most of the time, since
    # start_polling() is non-blocking and will stop the bot gracefully.
    updater.idle()


if __name__ == '__main__':
    main()