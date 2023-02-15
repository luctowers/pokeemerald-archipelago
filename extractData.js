const fs = require('fs')
const path = require('path')

const extractFlagValues = async () => {
  let lines = await fs.promises.readFile(path.join('.', 'include', 'constants', 'flags.h'), 'utf-8')
  lines = lines.split('\n')

  const flagNameToValue = {
    MAX_TRAINERS_COUNT: 864
  }

  lines.forEach((line) => {
    const match = line.match(/#define ([A-Z0-9x_]+)[ \t]+(?:(.*?)[ ]?)(?=(?:\/\/|$))/)
    if (match !== null) {
      let [_m, flagName, valueString, _comment] = match

      const symbols = valueString.matchAll(/(?<![A-Zx_0-9])(?<![0-9x])[A-Zx_][A-Z0-9x_]+/g)
      ;[...symbols].forEach(([symbol]) => {
        valueString = valueString.replace(symbol, flagNameToValue[symbol])
      })

      const result = eval(valueString)
      if (Number.isNaN(result) || result === undefined) {
        throw new Error(`Failed to eval string: "${line}"`)
      }

      flagNameToValue[flagName] = result
    }
  })

  delete MAX_TRAINERS_COUNT

  return flagNameToValue
}

const extractItemValues = async () => {
  let lines = await fs.promises.readFile(path.join('.', 'include', 'constants', 'items.h'), 'utf-8')
  lines = lines.split('\n')

  const itemNameToValue = {}

  lines.forEach((line) => {
    const match = line.match(/#define ([A-Z0-9x_]+)[ \t]+(?:(.*?)[ ]?)(?=(?:\/\/|$))/)
    if (match !== null) {
      let [_m, itemName, valueString, _comment] = match

      const symbols = valueString.matchAll(/(?<![A-Zx_0-9])(?<![0-9x])[A-Zx_][A-Z0-9x_]+/g)
      ;[...symbols].forEach(([symbol]) => {
        valueString = valueString.replace(symbol, itemNameToValue[symbol])
      })

      const result = eval(valueString)
      if (Number.isNaN(result) || result === undefined) {
        throw new Error(`Failed to eval string: "${line}"`)
      }

      itemNameToValue[itemName] = result
    }
  })

  return itemNameToValue
}

const extractMapData = async () => {
  const mapFilePaths = (await fs.promises.readdir(path.join('.', 'data', 'maps'), { withFileTypes: true }))
    .filter((dirent) => dirent.isDirectory())
    .map((dirent) => path.join('.', 'data', 'maps', dirent.name, 'map.json'))
  
  const ballItems = {}
  const hiddenItems = {}
  for (const mapFilePath of mapFilePaths) {
    const data = JSON.parse(await fs.promises.readFile(mapFilePath, 'utf-8'))

    // Ball Items
    // Filter against graphics_id for voltorb traps?
    data.object_events?.filter((event) => event.flag.match(/^FLAG_ITEM.+/) !== null)
      ?.forEach((event) => ballItems[event.flag.substring('FLAG_'.length)] = { flagName: event.flag, scriptSymbol: event.script })

    // Hidden Items
    data.bg_events?.filter((event) => event.type === 'hidden_item')
      ?.forEach((event) => hiddenItems[event.flag.substring('FLAG_'.length)] = { flagName: event.flag, itemName: event.item })
  }

  return {
    ballItems,
    hiddenItems
  }
}

const globalVariables = [
  'gSaveBlock1Ptr',
  'gArchipelagoReceivedItem'
]
const createSymbolLocationRegex = (symbol) => new RegExp(`^[ ]*(0x[0-9a-fA-F]+)[ ]+${symbol}.*$`, 'm')

const extractSymbolAddresses = async (ballItems, hiddenItems) => {
  const output = {
    globals: {}
  }

  let lines = await fs.promises.readFile('./pokeemerald.map', 'utf-8')

  // Find ball item script addresses
  for (const locationName in ballItems) {
    const symbol = ballItems[locationName].scriptSymbol
    const match = lines.match(createSymbolLocationRegex(symbol))
    if (match === null) throw new Error(`Could not find symbol for string: ${symbol}`)
    ballItems[locationName].scriptAddress = Number.parseInt(match[1], 16) + 3
    delete ballItems.scriptSymbol
  }

  // Find hidden item addresses from injected symbols
  for (const locationName in hiddenItems) {
    const symbol = 'Archipelago_Target_' + hiddenItems[locationName].flagName
    const match = lines.match(createSymbolLocationRegex(symbol))
    if (match === null) throw new Error(`Could not find symbol for string: ${symbol}`)
    hiddenItems[locationName].itemAddress = Number.parseInt(match[1], 16) + 8
  }

  // Find addresses of specified symbols
  globalVariables.forEach((symbol) => {
    const match = lines.match(createSymbolLocationRegex(symbol))
    if (match === null) throw new Error(`Could not find symbol for string: ${symbol}`)
    output.globals[symbol] = Number.parseInt(match[1], 16)
  })

  return output
}

;(async () => {
  const flagNameToValue = await extractFlagValues()
  const itemNameToValue = await extractItemValues()
  const { ballItems, hiddenItems } = await extractMapData()
  const { globals } = await extractSymbolAddresses(ballItems, hiddenItems)

  const output = {
    globals,
    ballItems,
    hiddenItems,
    flagNameToValue,
    itemNameToValue
  }

  await fs.promises.writeFile('./romData.json', JSON.stringify(output), 'utf-8')
})()
