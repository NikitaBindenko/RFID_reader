import asyncio
import serial

from aiogram import Bot, Dispatcher, executor
from config import BOT_TOKEN

file = open("D:/Python/telegrambot/users.txt", "r")
list_of_users = set()
for line in file:
  list_of_users.add(line.strip())
file.close()
loop = asyncio.get_event_loop()
bot = Bot(BOT_TOKEN, parse_mode="HTML")
dp = Dispatcher(bot, loop=loop)
if __name__ == "__main__":
   from handlers import dp, save_id, send_info
   executor.start_polling(dp)


